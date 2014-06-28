#include <assert.h>

#include <array>
#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
		
		bool isRootTypeList(const std::vector<SEM::Type*>& templateArguments);
		
		bool isRootType(SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::OBJECT: {
					return isRootTypeList(type->templateArguments());
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
		
		TypePair pathType(Module& module) {
			return std::make_pair(llvm_abi::Type::Integer(module.abiContext(), llvm_abi::Int32),
				TypeGenerator(module).getI32Type());
		}
		
		llvm::StructType* templateGeneratorLLVMType(Module& module) {
			const auto name = "__template_generator";
			
			const auto result = module.getTypeMap().tryGet(name);
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			const auto structType = typeGen.getForwardDeclaredStructType(name);
			
			module.getTypeMap().insert(name, structType);
			
			llvm::SmallVector<llvm::Type*, 2> structMembers;
			structMembers.push_back(typeGen.getI8PtrType());
			structMembers.push_back(typeGen.getI32Type());
			structType->setBody(structMembers);
			return structType;
		}
		
		llvm_abi::Type* templateGeneratorABIType(Module& module) {
			auto& abiContext = module.abiContext();
			llvm::SmallVector<llvm_abi::Type*, 2> types;
			types.push_back(llvm_abi::Type::Pointer(abiContext));
			types.push_back(llvm_abi::Type::Integer(abiContext, llvm_abi::Int32));
			return llvm_abi::Type::AutoStruct(abiContext, types);
		}
		
		TypePair templateGeneratorType(Module& module) {
			const auto iterator = module.standardTypeMap().find(TemplateGeneratorType);
			if (iterator != module.standardTypeMap().end()) {
				return iterator->second;
			}
			
			const auto type = std::make_pair(templateGeneratorABIType(module), templateGeneratorLLVMType(module));
			module.standardTypeMap().insert(std::make_pair(TemplateGeneratorType, type));
			return type;
		}
		
		llvm::Type* typeInfoLLVMType(Module& module) {
			TypeGenerator typeGen(module);
			const auto name = "__type_info";
			const auto structType = typeGen.getForwardDeclaredStructType(name);
			
			llvm::SmallVector<llvm::Type*, 2> structMembers;
			structMembers.push_back(vtableType(module)->getPointerTo());
			structMembers.push_back(templateGeneratorType(module).second);
			structType->setBody(structMembers);
			return structType;
		}
		
		llvm_abi::Type* typeInfoABIType(Module& module) {
			auto& abiContext = module.abiContext();
			llvm::SmallVector<llvm_abi::Type*, 2> types;
			types.push_back(llvm_abi::Type::Pointer(abiContext));
			types.push_back(templateGeneratorABIType(module));
			return llvm_abi::Type::AutoStruct(abiContext, types);
		}
		
		TypePair typeInfoType(Module& module) {
			const auto iterator = module.standardTypeMap().find(TypeInfoType);
			if (iterator != module.standardTypeMap().end()) {
				return iterator->second;
			}
			
			const auto type = std::make_pair(typeInfoABIType(module), typeInfoLLVMType(module));
			module.standardTypeMap().insert(std::make_pair(TypeInfoType, type));
			return type;
		}
		
		TypePair typeInfoArrayType(Module& module) {
			const auto typeInfo = typeInfoType(module);
			auto& abiContext = module.abiContext();
			TypeGenerator typeGen(module);
			return std::make_pair(llvm_abi::Type::Array(abiContext, TYPE_INFO_ARRAY_SIZE, typeInfo.first),
				typeGen.getArrayType(typeInfo.second, TYPE_INFO_ARRAY_SIZE));
		}
		
		ArgInfo rootFunctionArgInfo(Module& module) {
			llvm::SmallVector<TypePair, 1> types;
			types.push_back(pathType(module));
			return ArgInfo::Basic(module, typeInfoArrayType(module), types);
		}
		
		llvm::Value* computeTemplateArguments(Function& function, llvm::Value* generatorValue) {
			auto& builder = function.getBuilder();
			
			const auto generatorRootFn = builder.CreateExtractValue(generatorValue, { 0 }, "rootFn");
			const auto generatorPath = builder.CreateExtractValue(generatorValue, { 1 }, "path");
			
			const auto argInfo = rootFunctionArgInfo(function.module());
			const auto functionType = argInfo.makeFunctionType();
			const auto castRootFn = builder.CreateBitCast(generatorRootFn, functionType->getPointerTo(), "castRootFn");
			
			const bool canThrow = false;
			const auto callResult = genRawFunctionCall(function, argInfo, canThrow, castRootFn, { generatorPath });
			callResult->setName("templateArgs");
			
			// TODO: add to ArgInfo:
			// callInstruction->setDoesNotAccessMemory();
			
			return callResult;
		}
		
		llvm::Value* nullTemplateGenerator(Module& module) {
			ConstantGenerator constGen(module);
			
			llvm::SmallVector<llvm::Constant*, 2> values;
			values.resize(2);
			values[0] = constGen.getNull(TypeGenerator(module).getI8PtrType());
			values[1] = constGen.getI32(0);
			return constGen.getStruct(templateGeneratorLLVMType(module), values);
		}
		
		llvm::Value* computeTemplateGenerator(Function& function, SEM::Type* type) {
			assert(type->isObject());
			assert(!type->isInterface());
			
			auto& module = function.module();
			auto& builder = function.getEntryBuilder();
			
			ConstantGenerator constGen(module);
			
			if (type->templateArguments().empty()) {
				// If the type doesn't have any template arguments, then
				// provide a 'null' root function.
				return nullTemplateGenerator(module);
			} else if (hasVirtualTypeArgument(type)) {
				// If virtual type arguments are provided
				// then set the 'null' template generator,
				// since the template arguments should already
				// be stored inside the object.
				return nullTemplateGenerator(module);
			} else if (isRootTypeList(type->templateArguments())) {
				// If there are only 'root' arguments (i.e. statically known),
				// generate a root generator function for them and use
				// path of '1' to only pick up the top level argument values.
				const auto rootFunction = genTemplateRootFunction(module, type);
				
				llvm::SmallVector<llvm::Constant*, 2> values;
				values.resize(2);
				values[0] = constGen.getPointerCast(rootFunction, TypeGenerator(module).getI8PtrType());
				values[1] = constGen.getI32(1);
				return constGen.getStruct(templateGeneratorLLVMType(module), values);
			} else {
				auto& templateBuilder = function.templateBuilder();
				
				// Add a use to the parent type's intermediate template generator.
				const auto entryId = templateBuilder.addUse(type);
				
				const auto parentTemplateGenerator = function.getTemplateGenerator();
				const auto rootFunction = builder.CreateExtractValue(parentTemplateGenerator, { 0 });
				const auto path = builder.CreateExtractValue(parentTemplateGenerator, { 1 });
				
				// Insert garbage value for bits required.
				const auto bitsRequiredGarbageValue = constGen.getI32(1000);
				
				const auto shiftedPath = builder.CreateShl(path, bitsRequiredGarbageValue);
				
				// Record the instruction so it can be modified later when we
				// actually know how many intermediate template uses exist (and
				// hence know the bits required).
				templateBuilder.addInstruction(llvm::dyn_cast<llvm::Instruction>(shiftedPath));
				
				const auto newPath = builder.CreateOr(shiftedPath, constGen.getI32(entryId));
				
				llvm::Value* templateGenerator = constGen.getUndef(templateGeneratorType(module).second);
				templateGenerator = builder.CreateInsertValue(templateGenerator, rootFunction, { 0 });
				templateGenerator = builder.CreateInsertValue(templateGenerator, newPath, { 1 });
				return templateGenerator;
			}
		}
		
		llvm::Value* getTemplateGenerator(Function& function, SEM::Type* type) {
			assert(type->isObject());
			assert(!type->isInterface());
			
			const auto iterator = function.templateGeneratorMap().find(type);
			if (iterator != function.templateGeneratorMap().end()) {
				return iterator->second;
			}
			
			const auto value = computeTemplateGenerator(function, type);
			function.templateGeneratorMap().insert(std::make_pair(type, value));
			return value;
		}
		
		llvm::Function* genTemplateRootFunction(Module& module, SEM::Type* type) {
			assert(isRootType(type));
			
			const auto iterator = module.templateRootFunctionMap().find(type);
			if (iterator != module.templateRootFunctionMap().end()) {
				return iterator->second;
			}
			
			const auto argInfo = rootFunctionArgInfo(module);
			const auto llvmFunction = createLLVMFunction(module, argInfo.makeFunctionType(), llvm::Function::PrivateLinkage, "template_root");
			llvmFunction->setDoesNotAccessMemory();
			llvmFunction->setDoesNotThrow();
			
			module.templateRootFunctionMap().insert(std::make_pair(type, llvmFunction));
			
			// Always inline template generators.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			Function function(module, *llvmFunction, argInfo);
			
			auto& builder = function.getBuilder();
			
			const auto pathArg = function.getArg(0);
			
			ConstantGenerator constGen(module);
			llvm::Value* newTypesValue = constGen.getUndef(typeInfoArrayType(module).second);
			
			for (size_t i = 0; i < type->templateArguments().size(); i++) {
				const auto& templateArg = type->templateArguments().at(i);
				const auto vtablePointer = genVTable(module, templateArg);
				
				// Create type info struct.
				llvm::Value* typeInfo = constGen.getUndef(typeInfoType(module).second);
				typeInfo = builder.CreateInsertValue(typeInfo, vtablePointer, { 0 });
				
				const auto generator = getTemplateGenerator(function, templateArg);
				typeInfo = builder.CreateInsertValue(typeInfo, generator, { 1 });
				
				newTypesValue = builder.CreateInsertValue(newTypesValue, typeInfo, { (unsigned int) i });
			}
			
			if (type->isPrimitive()) {
				// Primitives don't have template generators;
				// the path must have ended by now.
				builder.CreateRet(newTypesValue);
				return llvmFunction;
			}
			
			// If the path is 1 (i.e. just the leading 1), that means that the
			// path terminates immediately so just return the new types struct.
			// Otherwise, call the next intermediate template generator.
			const auto pathEndBB = function.createBasicBlock("pathEnd");
			const auto callNextGeneratorBB = function.createBasicBlock("callNextGenerator");
			
			const auto compareValue = builder.CreateICmpEQ(pathArg, constGen.getI32(1));
			builder.CreateCondBr(compareValue, pathEndBB, callNextGeneratorBB);
			
			function.selectBasicBlock(pathEndBB);
			builder.CreateRet(newTypesValue);
			
			function.selectBasicBlock(callNextGeneratorBB);
			
			TypeGenerator typeGen(module);
			
			llvm::Type* const ctlzTypes[] = { typeGen.getI32Type() };
			const auto countLeadingZerosFunction = llvm::Intrinsic::getDeclaration(function.module().getLLVMModulePtr(), llvm::Intrinsic::ctlz, ctlzTypes);
			llvm::Value* const ctlzArgs[] = { pathArg, constGen.getI1(true) };
			const auto numLeadingZeroes = builder.CreateCall(countLeadingZerosFunction, ctlzArgs);
			const auto numLeadingZeroesI8 = builder.CreateTrunc(numLeadingZeroes, typeGen.getI8Type());
			const auto startPosition = builder.CreateSub(constGen.getI8(31), numLeadingZeroesI8);
			
			const auto nextFunction = genTemplateIntermediateFunctionDecl(module, type->getObjectType());
			
			const bool canThrow = false;
			llvm::Value* const args[] = { newTypesValue, llvmFunction, pathArg, startPosition };
			builder.CreateRet(genRawFunctionCall(function, intermediateFunctionArgInfo(module), canThrow, nextFunction, args));
			
			return llvmFunction;
		}
		
		ArgInfo intermediateFunctionArgInfo(Module& module) {
			auto& abiContext = module.abiContext();
			
			std::vector<TypePair> argTypes;
			argTypes.reserve(4);
			
			argTypes.push_back(typeInfoArrayType(module));
			
			const auto rootFunctionType = rootFunctionArgInfo(module).makeFunctionType();
			argTypes.push_back(std::make_pair(llvm_abi::Type::Pointer(abiContext), rootFunctionType->getPointerTo()));
			
			argTypes.push_back(pathType(module));
			argTypes.push_back(std::make_pair(llvm_abi::Type::Integer(abiContext, llvm_abi::Int8), TypeGenerator(module).getI8Type()));
			
			return ArgInfo::Basic(module, typeInfoArrayType(module), argTypes);
		}
		
		llvm::Function* genTemplateIntermediateFunctionDecl(Module& module, SEM::TypeInstance* typeInstance) {
			assert(!typeInstance->isPrimitive());
			const auto mangledName = mangleTemplateGenerator(typeInstance);
			
			const auto result = module.getFunctionMap().tryGet(mangledName);
			if (result.hasValue()) {
				return result.getValue();
			}
			
			const auto functionType = intermediateFunctionArgInfo(module).makeFunctionType();
			const auto llvmFunction = createLLVMFunction(module, functionType, getFunctionLinkage(typeInstance, typeInstance->moduleScope()), mangledName);
			llvmFunction->setDoesNotAccessMemory();
			llvmFunction->setDoesNotThrow();
			
			module.getFunctionMap().insert(mangledName, llvmFunction);
			
			return llvmFunction;
		}
		
		llvm::Function* genTemplateIntermediateFunction(Module& module, SEM::TypeInstance* parentType, const TemplateBuilder& templateBuilder) {
			ConstantGenerator constGen(module);
			
			const auto argInfo = intermediateFunctionArgInfo(module);
			const auto llvmFunction = genTemplateIntermediateFunctionDecl(module, parentType);
			
			if (parentType->isClassDecl()) {
				return llvmFunction;
			}
			
			Function function(module, *llvmFunction, argInfo);
			
			// Always inline template generators.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			const auto typesArg = function.getArg(0);
			const auto rootFnArg = function.getArg(1);
			const auto pathArg = function.getArg(2);
			const auto parentPositionArg = function.getArg(3);
			
			auto& builder = function.getBuilder();
			
			const auto position = builder.CreateSub(parentPositionArg, constGen.getI8(templateBuilder.bitsRequired()));
			const auto castPosition = builder.CreateZExt(position, TypeGenerator(module).getI32Type());
			const auto subPath = builder.CreateLShr(pathArg, castPosition);
			const auto mask = constGen.getI32((1 << templateBuilder.bitsRequired()) - 1);
			const auto component = builder.CreateAnd(subPath, mask);
			
			// Generate the component entry for each template use.
			for (const auto& templateUsePair: templateBuilder.templateUseMap()) {
				const auto& templateUseType = templateUsePair.first;
				const auto templateUseComponent = templateUsePair.second;
				
				const auto foundComponentEntryBB = function.createBasicBlock("foundComponentEntry");
				const auto tryNextComponentEntryBB = function.createBasicBlock("tryNextComponentEntry");
				const auto componentCompareValue = builder.CreateICmpEQ(component, constGen.getI32(templateUseComponent));
				builder.CreateCondBr(componentCompareValue, foundComponentEntryBB, tryNextComponentEntryBB);
				
				function.selectBasicBlock(foundComponentEntryBB);
				
				llvm::Value* newTypesValue = constGen.getUndef(typeInfoArrayType(module).second);
				
				// Loop through each template argument and generate it.
				for (size_t i = 0; i < templateUseType->templateArguments().size(); i++) {
					const auto& templateUseArg = templateUseType->templateArguments().at(i);
					if (templateUseArg->isTemplateVar()) {
						// For template variables, just copy across the existing type
						// from the types provided to us by the caller.
						const auto templateVarIndex = templateUseArg->getTemplateVar()->index();
						const auto templateVarValue = builder.CreateExtractValue(typesArg, templateVarIndex);
						newTypesValue = builder.CreateInsertValue(newTypesValue, templateVarValue, { (unsigned int) i });
					} else {
						// For an object type need to obtain the vtable (and potentially
						// also the generator function for its template arguments).
						const auto vtablePointer = genVTable(module, templateUseArg);
						
						llvm::Value* typeInfo = constGen.getUndef(typeInfoType(module).second);
						typeInfo = builder.CreateInsertValue(typeInfo, vtablePointer, { 0 });
						
						if (templateUseArg->templateArguments().empty()) {
							// If the type doesn't have any template arguments, then
							// provide a 'null' template generator.
							typeInfo = builder.CreateInsertValue(typeInfo, nullTemplateGenerator(module), { 1 });
						} else {
							// If there are arguments, refer to the component for them
							// by adding the correct component in the path by computing
							// (subPath & ~mask) | <their component>.
							const auto castRootFunction = builder.CreatePointerCast(rootFnArg, TypeGenerator(module).getI8PtrType());
							const auto argComponent = templateBuilder.templateUseMap().at(templateUseArg);
							const auto maskedSubPath = builder.CreateAnd(subPath, builder.CreateNot(mask));
							const auto argFullPath = builder.CreateOr(maskedSubPath, constGen.getI32(argComponent));
							
							llvm::Value* templateGenerator = constGen.getUndef(templateGeneratorType(module).second);
							templateGenerator = builder.CreateInsertValue(templateGenerator, castRootFunction, { 0 });
							templateGenerator = builder.CreateInsertValue(templateGenerator, argFullPath, { 1 });
							typeInfo = builder.CreateInsertValue(typeInfo, templateGenerator, { 1 });
						}
						
						newTypesValue = builder.CreateInsertValue(newTypesValue, typeInfo, { (unsigned int) i });
					}
				}
				
				if (templateUseType->isPrimitive()) {
					// Primitives don't have template generators;
					// the path must have ended by now.
					builder.CreateRet(newTypesValue);
				} else {
					// If the position is 0, that means we've reached the end of the path,
					// so just return the new types struct. Otherwise, call the next
					// intermediate template generator.
					const auto pathEndBB = function.createBasicBlock("pathEnd");
					const auto callNextGeneratorBB = function.createBasicBlock("callNextGenerator");
					
					const auto compareValue = builder.CreateICmpEQ(position, constGen.getI8(0));
					builder.CreateCondBr(compareValue, pathEndBB, callNextGeneratorBB);
					
					function.selectBasicBlock(pathEndBB);
					builder.CreateRet(newTypesValue);
					
					function.selectBasicBlock(callNextGeneratorBB);
					
					// Call the next intermediate function.
					const auto nextFunction = genTemplateIntermediateFunctionDecl(module, templateUseType->getObjectType());
					
					const bool canThrow = false;
					const auto callResult = genRawFunctionCall(function, argInfo, canThrow, nextFunction, std::vector<llvm::Value*>{ newTypesValue, rootFnArg, pathArg, position });
					builder.CreateRet(callResult);
				}
				
				function.selectBasicBlock(tryNextComponentEntryBB);
			}
			
			// Path is not valid...
			builder.CreateUnreachable();
			
			return llvmFunction;
		}
		
	}
	
}

