#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TargetInfo.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
		
		bool isRootType(SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::VOID: {
					return true;
				}
				
				case SEM::Type::OBJECT: {
					return isRootTypeList(type->templateArguments());
				}
				
				case SEM::Type::REFERENCE: {
					// TODO?
					return true;
				}
				
				case SEM::Type::FUNCTION: {
					return isRootType(type->getFunctionReturnType()) && isRootTypeList(type->getFunctionParameterTypes());
				}
				
				case SEM::Type::METHOD: {
					return isRootType(type->getMethodFunctionType());
				}
				
				case SEM::Type::INTERFACEMETHOD: {
					return isRootType(type->getInterfaceMethodFunctionType());
				}
				
				case SEM::Type::TEMPLATEVAR: {
					return false;
				}
				
				default: {
					llvm_unreachable("Unknown SEM::Type kind in isRootType()");
				}
			}
		}
		
		bool isRootTypeList(const std::vector<SEM::Type*>& templateArguments) {
			for (const auto arg: templateArguments) {
				if (!isRootType(arg)) return false;
			}
			return true;
		}
		
		constexpr size_t TYPE_INFO_ARRAY_SIZE = 8;
		
		llvm::Type* typeInfoType(Module& module) {
			TypeGenerator typeGen(module);
			return typeGen.getStructType({ typeGen.getI8PtrType(), typeGen.getI8PtrType(), typeGen.getI32Type() });
		}
		
		llvm::Type* typeInfoArrayType(Module& module) {
			TypeGenerator typeGen(module);
			return typeGen.getArrayType(TYPE_INFO_ARRAY_SIZE, typeInfoType(module));
		}
		
		std::vector<llvm_abi::Type> rootFunctionABIArgumentTypes() {
			return { llvm_abi::Type::Integer(llvm_abi::Int32) };
		}
		
		std::vector<llvm::Type*> rootFunctionArgumentTypes(Module& module) {
			TypeGenerator typeGen(module);
			return std::vector<llvm::Type*>{ typeGen.getI32Type() };
		}
		
		llvm::FunctionType* rootFunctionType(Module& module) {
			// TODO: rewrite the function type according to the ABI.
			TypeGenerator typeGen(module);
			return typeGen.getFunctionType(typeInfoArrayType(module), rootFunctionArgumentTypes(module));
		}
		
		llvm::Value* computeTemplateArguments(Function& function, llvm::Value* generatorValue) {
			auto& builder = function.getBuilder();
			
			const auto generatorRootFn = builder.CreateExtractValue(generatorValue, { 0 }, "rootFn");
			const auto generatorPath = builder.CreateExtractValue(generatorValue, { 0 }, "path");
			
			const auto functionType = rootFunctionType(function.module());
			const auto castRootFn = builder.CreateBitcast(generatorRootFn, functionType->getPointerTo(), "castRootFn");
			const auto callInstruction = builder.CreateCall(castRootFn, { generatorPath }, "templateArgs");
			callInstruction->setDoesNotAccessMemory();
			return callInstruction;
		}
		
		llvm::Value* computeTemplateGenerator(Function& function, SEM::Type* type) {
			assert(!type->templateArguments().empty());
			
			auto& module = function.module();
			auto& builder = function.getBuilder();
			ConstantGenerator constGen(module);
			
			if (isRootTypeList(type->templateArguments())) {
				const auto rootFunction = genTemplateRootFunction(module, type);
				llvm::Value* typeInfo = constGen.createUndef(templateGeneratorType(module));
				typeInfo = builder.CreateInsertValue(typeInfo, rootFunction, { 0 });
				typeInfo = builder.CreateInsertValue(typeInfo, constGen.getI8(1), { 0 });
				return typeInfo;
			} else {
				const auto entryId = function.addIntermediateTemplateUse(type);
				// TODO...
			}
		}
		
		llvm::Function* genTemplateRootFunction(Module& module, SEM::Type* type) {
			assert(isRootTypeList(templateArguments));
			
			const auto llvmFunction = createLLVMFunction(module, rootFunctionType(module), llvm::Function::PrivateLinkage, NO_FUNCTION_NAME);
			llvmFunction->setDoesNotAccessMemory();
			
			Function function(module, llvmFunction, ArgInfo::Basic(rootFunctionABIArgumentTypes(), rootFunctionArgumentTypes(module)));
			
			llvm::Value* newTypesValue = constGen.getUndef(typeInfoArrayType);
			
			for (size_t i = 0; i < type->templateArguments().size(); i++) {
				const auto& templateArg = type->templateArguments().at(i);
				const auto vtablePointer = genVTable(module, templateArg->getObjectType());
				
				// Create { vtablePtr, rootFn, path } type info struct.
				llvm::Value* typeInfo = constGen.createUndef(typeInfoType(module));
				typeInfo = builder.CreateInsertValue(typeInfo, vtablePointer, { 0 });
				
				if (templateArg->templateArguments().empty()) {
					// If the type doesn't have any template arguments, then
					// provide a 'null' root function.
					typeInfo = builder.CreateInsertValue(typeInfo, constGen.getNull(rootFunctionType(module)), { 1 });
					typeInfo = builder.CreateInsertValue(typeInfo, constGen.getI32(0), { 2 });
				} else {
					// If there are arguments, also generate a root generate
					// function for them and use path of '1' to only pick up
					// the top level argument values.
					const auto subRootFunction = genTemplateRootFunction(module, templateArg);
					typeInfo = builder.CreateInsertValue(typeInfo, subRootFunction, { 1 });
					typeInfo = builder.CreateInsertValue(typeInfo, constGen.getI32(1), { 2 });
				}
				
				newTypesValue = builder.CreateInsertValue(newTypesValue, typeInfo);
			}
			
			const auto countLeadingZerosFunction = llvm::Intrinsic::getDeclaration(function.module().getLLVMModulePtr(), llvm::Intrinsic::ctlz);
			const auto startPosition = builder.CreateSub(constGen.getI8(31), builder.CreateCall(countLeadingZerosFunction, { pathArg }));
			
			const auto nextFunction = genTemplateIntermediateFunctionDecl(module, type->getObjectType());
			builder.CreateRet(builder.CreateCall(nextFunction, { newTypesValue, llvmFunction, pathArg, startPosition }));
			
			return llvmFunction;
		}
		
		inline uint8_t getNextPowerOfTwo(size_t value) {
			uint8_t power = 0;
			while (value != 0) {
				value >>= 1;
				power++;
			}
			return power;
		}
		
		void addUseSet(size_t& currentId, std::map<SEM::Type*, size_t>& currentMap, SEM::Type* type) {
			assert(type->isObjectType());
			
			if (arg->templateArguments.empty()) {
				// Don't add entries for types with no template arguments.
				return;
			}
			
			// Add a component entry for this type.
			currentMap.insert(std::make_pair(type, currentId++));
			
			for (const auto& arg: type->templateArguments()) {
				if (arg->isObjectType()) {
					addUseSet(currentId, currentMap, arg);
				}
			}
		}
		
		std::map<SEM::Type*, size_t> createUseMap(const std::vector<SEM::Type*>& templateUses) {
			size_t currentId = 0;
			std::map<SEM::Type*, size_t> currentMap;
			for (const auto arg: templateUses) {
				addUseSet(currentId, currentMap, arg);
			}
			return currentMap;
		}
		
		std::vector<llvm_abi::Type> intermediateFunctionABIArgumentTypes() {
			// Type[8] types, void* rootFn, uint32_t path, uint8_t position
			std::vector<llvm_abi::Type> argumentTypes;
			argumentTypes.push_back(llvm_abi::Type::Array(TYPE_INFO_ARRAY_SIZE, typeInfoABIType()));
			argumentTypes.push_back(llvm_abi::Type::Pointer());
			argumentTypes.push_back(llvm_abi::Type::Integer(llvm_abi::Int32));
			argumentTypes.push_back(llvm_abi::Type::Integer(llvm_abi::Int8));
			return argumentTypes;
		}
		
		std::vector<llvm::Type*> intermediateFunctionArgumentTypes(Module& module) {
			// Type[8] types, void* rootFn, uint32_t path, uint8_t position
			TypeGenerator typeGen(module);
			return std::vector<llvm::Type*>{ typeInfoArrayType(module), typeGen.getI8PtrType(), typeGen.getI32Type(), typeGen.getI8Type() };
		}
		
		llvm::FunctionType* intermediateFunctionType(Module& module) {
			// TODO: rewrite the function type according to the ABI.
			TypeGenerator typeGen(module);
			return typeGen.getFunctionType(typeInfoArrayType(module), intermediateFunctionArgumentTypes(module));
		}
		
		llvm::Function* genTemplateIntermediateFunctionDecl(Module& module, SEM::TypeInstance* typeInstance) {
			const auto mangledName = mangleTemplateGenerator(typeInstance);
			
			const auto result = module.getFunctionMap().tryGet(mangledName);
			if (result.hasValue()) {
				return result.getValue();
			}
			
			const auto llvmFunction = createLLVMFunction(module, intermediateFunctionType(module), llvm::Function::ExternalLinkage, mangledName);
			llvmFunction->setDoesNotAccessMemory();
			return llvmFunction;
		}
		
		llvm::Function* genTemplateIntermediateFunction(Module& module, SEM::TypeInstance* parentType, const std::map<SEM::Type*, size_t>& templateUses) {
			// Get the number of bits that must be assigned in the
			// path for determining the relevant component entry.
			const auto bitsRequired = getNextPowerOfTwo(templateUses.size());
			
			ConstantGenerator constGen(module);
			
			const auto llvmFunction = genTemplateIntermediateFunctionDecl(module, parentType);
			Function function(module, llvmFunction, ArgInfo::Basic(intermediateFunctionABIArgumentTypes(), intermediateFunctionArgumentTypes(module)));
			
			const auto typesArg = function.getArg(0);
			const auto rootFnArg = function.getArg(1);
			const auto pathArg = function.getArg(2);
			const auto positionArg = function.getArg(3);
			
			auto& builder = function.getBuilder();
			
			const auto pathEndBB = function.createBasicBlock("pathEnd");
			const auto processSubpathBB = function.createBasicBlock("processSubpath");
			
			const auto compareValue = builder.CreateICmpEq(position, constGen.getI8(0));
			builder.CreateCondBr(compareValue, pathEndBB, processSubpathBB);
			
			function.selectBasicBlock(pathEndBB);
			builder.CreateRet(typesArg);
			
			function.selectBasicBlock(processSubpathBB);
			
			const auto subPath = builder.CreateLShr(pathArg, positionArg);
			const auto component = builder.CreateAnd(subPath, constGen.getI32((1 << bitsRequired) - 1));
			const auto mask = builder.CreateSub(builder.CreateShl(constGen.getI32(1), positionArg), constGen.getI32(1));
			const auto nextPosition = builder.CreateSub(positionArg, constGen.getI8(bitsRequired));
			
			// Generate the component entry for each template use.
			for (const auto& templateUsePair: templateUses) {
				const auto& templateUseType = templateUsePair.first;
				const auto templateUseComponent = templateUsePair.second;
				
				const auto foundComponentEntryBB = function.createBasicBlock("foundComponentEntry");
				const auto tryNextComponentEntryBB = function.createBasicBlock("tryNextComponentEntry");
				const auto componentCompareValue = builder.CreateICmpEq(component, constGen.getI32(templateUseComponent));
				builder.CreateCondBr(componentCompareValue, foundComponentEntryBB, tryNextComponentEntryBB);
				
				function.selectBasicBlock(foundComponentEntryBB);
				
				const auto& templateUse = templateUses.at(i);
				
				llvm::Value* newTypesValue = constGen.getUndef(typeInfoArrayType);
				
				// Loop through each template argument and generate it.
				for (size_t i = 0; i < templateUseType->templateArguments().size(); i++) {
					const auto& templateUseArg = templateUse->templateArguments().at(i);
					if (templateUseArg->isTemplateVar()) {
						// For template variables, just copy across the existing type
						// from the types provided to us by the caller.
						const auto templateVarOffset = templateVarOffsetMap.at(templateUseArg.templateVar());
						const auto templateVarValue = builder.CreateExtractValue(typesArg, templateVarOffset);
						newTypesValue = builder.CreateInsertValue(newTypesValue, templateVarValue, { i });
					} else {
						// For an object type need to obtain the vtable (and potentially
						// also the generator function for its template arguments).
						const auto vtablePointer = genVTable(module, templateUseArg->getObjectType());
						
						llvm::Value* typeInfo = constGen.createUndef(typeInfoType);
						typeInfo = builder.CreateInsertValue(typeInfo, vtablePointer, { 0 });
						
						if (templateArg->templateArguments().empty()) {
							// If the type doesn't have any template arguments, then
							// provide a 'null' root function.
							typeInfo = builder.CreateInsertValue(typeInfo, constGen.getNull(rootFunctionType(module)), { 1 });
							typeInfo = builder.CreateInsertValue(typeInfo, constGen.getI32(0), { 2 });
						} else {
							// If there are arguments, refer to the component for them
							// by adding the correct component in the path by computing
							// (0x1 << (position + 2)) | (<their component> << position) | (mask & path).
							const auto argComponent = templateUses.at(templateUseArg);
							const auto pathStart = builder.CreateShl(constGen.getI32(1), nextPosition);
							const auto argSubPath = builder.CreateShl(constGen.getI32(argComponent), positionArg);
							const auto parentPath = builder.CreateAnd(pathArg, mask);
							const auto argFullPath = builder.CreateOr(pathStart, builder.CreateOr(argSubPath, parentPath));
							
							typeInfo = builder.CreateInsertValue(typeInfo, rootFnArg, { 1 });
							typeInfo = builder.CreateInsertValue(typeInfo, argFullPath, { 2 });
						}
						
						newTypesValue = builder.CreateInsertValue(newTypesValue, typeInfo, { i });
					}
				}
				
				// Call the next intermediate function.
				const auto nextFunction = genTemplateIntermediateFunctionDecl(module, templateUseType->getObjectType());
				builder.CreateRet(builder.CreateCall(nextFunction, { newTypesValue, rootFnArg, pathArg, nextPosition }));
				
				function.selectBasicBlock(tryNextComponentEntryBB);
			}
			
			// Path is not valid...
			builder.CreateUnreachable();
			
			return llvmFunction;
		}
		
	}
	
}

