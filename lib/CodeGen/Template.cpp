#include <assert.h>

#include <array>
#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/SEMFunctionGenerator.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
		
		bool isRootArgumentList(llvm::ArrayRef<SEM::Value> templateArguments);
		
		bool isRootType(const SEM::Type* const type) {
			switch (type->kind()) {
				case SEM::Type::OBJECT: {
					// Interface type template arguments don't affect
					// code generation in any way so they can be
					// ignored here.
					return type->getObjectType()->isInterface() ||
					       isRootArgumentList(arrayRef(type->templateArguments()));
				}
				case SEM::Type::TEMPLATEVAR: {
					return false;
				}
				case SEM::Type::ALIAS: {
					return isRootType(type->resolveAliases());
				}
				default: {
					llvm_unreachable("Unknown SEM::Type kind in isRootType()");
				}
			}
		}
		
		bool isRootArgument(const SEM::Value& value) {
			if (value.isTypeRef()) {
				return isRootType(value.typeRefType());
			} else if (value.isTemplateVarRef()) {
				return false;
			} else {
				return true;
			}
		}
		
		bool isRootArgumentList(llvm::ArrayRef<SEM::Value> templateArguments) {
			for (size_t i = 0; i < templateArguments.size(); i++) {
				if (!isRootArgument(templateArguments[i])) {
					return false;
				}
			}
			return true;
		}
		
		constexpr size_t TYPE_INFO_ARRAY_SIZE = 8;
		
		TypePair pathType(Module& module) {
			return std::make_pair(llvm_abi::Type::Integer(module.abiContext(), llvm_abi::Int32),
				TypeGenerator(module).getI32Type());
		}
		
		llvm::StructType* templateGeneratorLLVMType(Module& module) {
			const auto name = module.getCString("__template_generator");
			
			const auto iterator = module.getTypeMap().find(name);
			if (iterator != module.getTypeMap().end()) {
				return iterator->second;
			}
			
			TypeGenerator typeGen(module);
			const auto structType = typeGen.getForwardDeclaredStructType(name);
			
			module.getTypeMap().insert(std::make_pair(name, structType));
			
			llvm::SmallVector<llvm::Type*, 3> structMembers;
			structMembers.push_back(typeGen.getPtrType());
			structMembers.push_back(typeGen.getPtrType());
			structMembers.push_back(typeGen.getI32Type());
			structType->setBody(structMembers);
			return structType;
		}
		
		llvm_abi::Type* templateGeneratorABIType(Module& module) {
			auto& abiContext = module.abiContext();
			llvm::SmallVector<llvm_abi::Type*, 3> types;
			types.push_back(llvm_abi::Type::Pointer(abiContext));
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
			const auto name = module.getCString("__type_info");
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
			llvm::SmallVector<TypePair, 2> types;
			
			// Context pointer.
			types.push_back(pointerTypePair(module));
			
			// Path value.
			types.push_back(pathType(module));
			
			return ArgInfo::Basic(module, typeInfoArrayType(module), types).withNoMemoryAccess().withNoExcept();
		}
		
		llvm::Value* computeTemplateArguments(Function& function, llvm::Value* generatorValue) {
			auto& builder = function.getBuilder();
			
			const auto generatorRootFn = builder.CreateExtractValue(generatorValue, { 0 }, "rootFn");
			const auto contextPointer = builder.CreateExtractValue(generatorValue, { 1 }, "context");
			const auto generatorPath = builder.CreateExtractValue(generatorValue, { 2 }, "path");
			
			const auto argInfo = rootFunctionArgInfo(function.module());
			llvm::Value* const args[] = { contextPointer, generatorPath };
			const auto callResult = genRawFunctionCall(function, argInfo, generatorRootFn, args);
			callResult->setName("templateArgs");
			return callResult;
		}
		
		llvm::Constant* nullTemplateGenerator(Module& module) {
			ConstantGenerator constGen(module);
			
			llvm::Constant* const values[] = {
				constGen.getNull(TypeGenerator(module).getPtrType()),
				constGen.getNull(TypeGenerator(module).getPtrType()),
				constGen.getI32(0)
			};
			return constGen.getStruct(templateGeneratorLLVMType(module), values);
		}
		
		bool hasTemplateVirtualTypeArgument(llvm::ArrayRef<const SEM::Type*> arguments) {
			for (size_t i = 0; i < arguments.size(); i++) {
				if (arguments[i]->isInterface()) {
					return true;
				}
			}
			
			return false;
		}
		
		llvm::Value* computeTemplateGenerator(Function& function, const TemplateInst& templateInst) {
			auto& module = function.module();
			auto& builder = function.getEntryBuilder();
			
			ConstantGenerator constGen(module);
			IREmitter irEmitter(function);
			
			const auto typeInstance = templateInst.object().isTypeInstance() ? templateInst.object().typeInstance() : templateInst.object().parentTypeInstance();
			if (templateInst.arguments().empty() || (typeInstance != nullptr && typeInstance->isInterface())) {
				// If the type doesn't have any template arguments or we're
				// generating for an interface type (for which the arguments
				// are ignored in code generation) then provide a 'null'
				// root function.
				return nullTemplateGenerator(module);
			} else if (isRootArgumentList(templateInst.arguments())) {
				// If there are only 'root' arguments (i.e. statically known),
				// generate a root generator function for them and use
				// path of '1' to only pick up the top level argument values.
				const auto rootFunction = genTemplateRootFunction(function, templateInst);
				
				llvm::Constant* const values[] = {
						constGen.getPointerCast(rootFunction, TypeGenerator(module).getPtrType()),
						constGen.getNullPointer(TypeGenerator(module).getPtrType()),
						constGen.getI32(1)
					};
				return constGen.getStruct(templateGeneratorLLVMType(module), values);
			} else {
				auto& templateBuilder = function.templateBuilder();
				
				// Add a use to the parent type's intermediate template generator.
				const auto entryId = templateBuilder.addUse(templateInst);
				
				const auto parentTemplateGenerator = function.getTemplateGenerator();
				const auto rootFunction = builder.CreateExtractValue(parentTemplateGenerator, { 0 });
				const auto contextPointer = builder.CreateExtractValue(parentTemplateGenerator, { 1 });
				const auto path = builder.CreateExtractValue(parentTemplateGenerator, { 2 });
				
				// Insert garbage value for bits required; this will be replaced
				// later by the template builder when the actual number of bits
				// required is known.
				const auto bitsRequiredGarbageValue = constGen.getI32(4242);
				
				const auto shiftedPath = builder.CreateShl(path, bitsRequiredGarbageValue);
				
				// Record the instruction so it can be modified later when we
				// actually know how many intermediate template uses exist (and
				// hence know the bits required).
				templateBuilder.addInstruction(llvm::dyn_cast<llvm::Instruction>(shiftedPath));
				
				const auto newPath = builder.CreateOr(shiftedPath, constGen.getI32(entryId));
				
				SetUseEntryBuilder setUseEntryBuilder(function);
				
				llvm::Value* templateGenerator = constGen.getUndef(templateGeneratorType(module).second);
				templateGenerator = irEmitter.emitInsertValue(templateGenerator, rootFunction, { 0 });
				templateGenerator = irEmitter.emitInsertValue(templateGenerator, contextPointer, { 1 });
				templateGenerator = irEmitter.emitInsertValue(templateGenerator, newPath, { 2 });
				return templateGenerator;
			}
		}
		
		llvm::Value* getTemplateGenerator(Function& function, const TemplateInst& templateInst) {
			const auto iterator = function.templateGeneratorMap().find(templateInst);
			if (iterator != function.templateGeneratorMap().end()) {
				return iterator->second;
			}
			
			const auto value = computeTemplateGenerator(function, templateInst);
			function.templateGeneratorMap().insert(std::make_pair(templateInst.copy(), value));
			return value;
		}
		
		llvm::Function* genTemplateValueFunction(Function& parentFunction, const SEM::Value& value) {
			auto& module = parentFunction.module();
			
			SEM::FunctionAttributes attributes(/*isVarArg=*/false,
			                                   /*isMethod=*/false,
			                                   /*isTemplated=*/false,
			                                   /*noExceptPredicate=*/SEM::Predicate::True());
			SEM::FunctionType functionType(std::move(attributes),
			                               /*returnType=*/value.type(),
			                               /*parameterTypes=*/{});
			
			const auto argInfo = getFunctionArgInfo(module, functionType);
			
			const auto llvmFunction = createLLVMFunction(module, argInfo, llvm::Function::InternalLinkage, module.getCString("template_value"));
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			Function functionGenerator(module, *llvmFunction, argInfo);
			functionGenerator.attachDebugInfo(parentFunction.debugInfo());
			
			if (value.debugInfo()) {
				functionGenerator.setDebugPosition(value.debugInfo()->location.range().start());
			}
			
			const auto result = genValue(functionGenerator,
			                             value,
			                             functionGenerator.getReturnVarOrNull());
			
			if (argInfo.hasReturnVarArgument()) {
				genMoveStore(functionGenerator,
				             result,
				             functionGenerator.getReturnVar(),
				             value.type());
				functionGenerator.getBuilder().CreateRetVoid();
			} else if (!value.type()->isBuiltInVoid()) {
				functionGenerator.returnValue(result);
			} else {
				functionGenerator.getBuilder().CreateRetVoid();
			}
			
			return llvmFunction;
		}
		
		llvm::Function* genTemplateRootFunction(Function& parentFunction, const TemplateInst& templateInst) {
			assert(isRootArgumentList(templateInst.arguments()));
			
			auto& module = parentFunction.module();
			
			const auto iterator = module.templateRootFunctionMap().find(templateInst);
			if (iterator != module.templateRootFunctionMap().end()) {
				return iterator->second;
			}
			
			const auto argInfo = rootFunctionArgInfo(module);
			const auto llvmFunction = createLLVMFunction(module, argInfo, llvm::Function::InternalLinkage, module.getCString("template_root"));
			
			module.templateRootFunctionMap().insert(std::make_pair(templateInst.copy(), llvmFunction));
			
			// Always inline template generators.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			Function function(module, *llvmFunction, argInfo);
			IREmitter irEmitter(function);
			
			auto& builder = function.getBuilder();
			
			const auto contextPointerArg = function.getArg(0);
			const auto pathArg = function.getArg(1);
			
			ConstantGenerator constGen(module);
			llvm::Value* newTypesValue = constGen.getUndef(typeInfoArrayType(module).second);
			
			for (size_t i = 0; i < templateInst.arguments().size(); i++) {
				const auto& templateArg = templateInst.arguments()[i];
				if (templateArg.isTypeRef()) {
					const auto vtablePointer = genVTable(module, templateArg.typeRefType()->resolveAliases()->getObjectType());
					
					// Create type info struct.
					llvm::Value* typeInfo = constGen.getUndef(typeInfoType(module).second);
					typeInfo = irEmitter.emitInsertValue(typeInfo, vtablePointer, { 0 });
					
					const auto generator = getTemplateGenerator(function, TemplateInst::Type(templateArg.typeRefType()));
					typeInfo = irEmitter.emitInsertValue(typeInfo, generator, { 1 });
					
					newTypesValue = irEmitter.emitInsertValue(newTypesValue, typeInfo, { (unsigned int) i });
				} else {
					llvm::Value* typeInfo = constGen.getUndef(typeInfoType(module).second);
					
					const auto valueFunction = genTemplateValueFunction(parentFunction, templateArg);
					typeInfo = irEmitter.emitInsertValue(typeInfo, valueFunction, { 0 });
					
					const auto generator = nullTemplateGenerator(module);
					typeInfo = irEmitter.emitInsertValue(typeInfo, generator, { 1 });
					
					newTypesValue = irEmitter.emitInsertValue(newTypesValue, typeInfo, { (unsigned int) i });
				}
			}
			
			if (templateInst.object().isTypeInstance() && templateInst.object().typeInstance()->isPrimitive()) {
				// Primitives don't have template generators;
				// the path must have ended by now.
				irEmitter.emitReturn(typeInfoArrayType(module).second,
				                     newTypesValue);
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
			irEmitter.emitReturn(typeInfoArrayType(module).second,
			                     newTypesValue);
			
			function.selectBasicBlock(callNextGeneratorBB);
			
			TypeGenerator typeGen(module);
			
			llvm::Type* const ctlzTypes[] = { typeGen.getI32Type() };
			const auto countLeadingZerosFunction = llvm::Intrinsic::getDeclaration(function.module().getLLVMModulePtr(), llvm::Intrinsic::ctlz, ctlzTypes);
			llvm::Value* const ctlzArgs[] = { pathArg, constGen.getI1(true) };
			const auto numLeadingZeroes = builder.CreateCall(countLeadingZerosFunction, ctlzArgs);
			const auto numLeadingZeroesI8 = builder.CreateTrunc(numLeadingZeroes, typeGen.getI8Type());
			const auto startPosition = builder.CreateSub(constGen.getI8(31), numLeadingZeroesI8);
			
			const auto nextFunction = genTemplateIntermediateFunctionDecl(module, templateInst.object());
			
			llvm::Value* const args[] = { newTypesValue, llvmFunction, contextPointerArg, pathArg, startPosition };
			irEmitter.emitReturn(typeInfoArrayType(module).second,
			                     genRawFunctionCall(function, intermediateFunctionArgInfo(module), nextFunction, args));
			
			return llvmFunction;
		}
		
		ArgInfo intermediateFunctionArgInfo(Module& module) {
			auto& abiContext = module.abiContext();
			
			const auto rootFunctionType = rootFunctionArgInfo(module).makeFunctionType();
			
			std::vector<TypePair> argTypes;
			argTypes.reserve(5);
			
			// Already computed type info array.
			argTypes.push_back(typeInfoArrayType(module));
			
			// Root function pointer.
			argTypes.push_back(std::make_pair(llvm_abi::Type::Pointer(abiContext), rootFunctionType->getPointerTo()));
			
			// Root function context pointer.
			argTypes.push_back(pointerTypePair(module));
			
			// Path value.
			argTypes.push_back(pathType(module));
			
			// Position in path.
			argTypes.push_back(std::make_pair(llvm_abi::Type::Integer(abiContext, llvm_abi::Int8), TypeGenerator(module).getI8Type()));
			
			return ArgInfo::Basic(module, typeInfoArrayType(module), argTypes).withNoMemoryAccess().withNoExcept();
		}
		
		static llvm::GlobalValue::LinkageTypes
		getTemplatedObjectLinkage(Module& module,
		                          TemplatedObject templatedObject) {
			auto& semFunctionGenerator = module.semFunctionGenerator();
			if (templatedObject.isTypeInstance()) {
				return semFunctionGenerator.getTypeLinkage(*(templatedObject.typeInstance()));
			} else {
				return semFunctionGenerator.getLinkage(templatedObject.parentTypeInstance(),
				                                       *(templatedObject.function()));
			}
		}
		
		static bool isTemplatedObjectDecl(TemplatedObject templatedObject) {
			if (templatedObject.isTypeInstance()) {
				return templatedObject.typeInstance()->isClassDecl();
			} else {
				return templatedObject.function()->isDeclaration();
			}
		}
		
		llvm::Function* genTemplateIntermediateFunctionDecl(Module& module, TemplatedObject templatedObject) {
			assert(!templatedObject.isTypeInstance() || !templatedObject.typeInstance()->isPrimitive());
			const auto mangledName = mangleTemplateGenerator(module, templatedObject);
			
			const auto iterator = module.getFunctionMap().find(mangledName);
			if (iterator != module.getFunctionMap().end()) {
				return iterator->second;
			}
			
			const auto llvmFunction = createLLVMFunction(module,
			                                             intermediateFunctionArgInfo(module),
			                                             getTemplatedObjectLinkage(module, templatedObject),
			                                             mangledName);
			module.getFunctionMap().insert(std::make_pair(mangledName, llvmFunction));
			return llvmFunction;
		}
		
		llvm::Function* genTemplateIntermediateFunction(Module& module, TemplatedObject templatedObject, const TemplateBuilder& templateBuilder) {
			ConstantGenerator constGen(module);
			
			const auto argInfo = intermediateFunctionArgInfo(module);
			const auto llvmFunction = genTemplateIntermediateFunctionDecl(module, templatedObject);
			
			if (isTemplatedObjectDecl(templatedObject)) {
				return llvmFunction;
			}
			
			Function function(module, *llvmFunction, argInfo);
			
			// Always inline template generators.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			const auto typesArg = function.getArg(0);
			const auto rootFnArg = function.getArg(1);
			const auto rootContextArg = function.getArg(2);
			const auto pathArg = function.getArg(3);
			const auto parentPositionArg = function.getArg(4);
			
			IREmitter irEmitter(function);
			auto& builder = function.getBuilder();
			
			const auto position = builder.CreateSub(parentPositionArg, constGen.getI8(templateBuilder.bitsRequired()));
			const auto castPosition = builder.CreateZExt(position, TypeGenerator(module).getI32Type());
			const auto subPath = builder.CreateLShr(pathArg, castPosition);
			const auto mask = constGen.getI32((1 << templateBuilder.bitsRequired()) - 1);
			const auto component = builder.CreateAnd(subPath, mask);
			
			const auto endBB = function.createBasicBlock("");
			const auto switchInstruction = builder.CreateSwitch(component, endBB);
			
			// Generate the component entry for each template use.
			for (const auto& templateUsePair: templateBuilder.templateUseMap()) {
				const auto& templateUseInst = templateUsePair.first;
				const auto templateUseComponent = templateUsePair.second;
				
				const auto caseBB = function.createBasicBlock("");
				switchInstruction->addCase(constGen.getI32(templateUseComponent), caseBB);
				
				function.selectBasicBlock(caseBB);
				
				llvm::Value* newTypesValue = constGen.getUndef(typeInfoArrayType(module).second);
				
				// Loop through each template argument and generate it.
				for (size_t i = 0; i < templateUseInst.arguments().size(); i++) {
					if (!templateUseInst.arguments()[i].isTypeRef()) {
						if (templateUseInst.arguments()[i].isTemplateVarRef()) {
							// Template var ref values just need to
							// propagate the relevant argument.
							const auto templateUseVar = templateUseInst.arguments()[i].templateVar();
							const auto templateVarIndex = templateUseVar->index();
							const auto templateRefEntryValue = builder.CreateExtractValue(typesArg, templateVarIndex);
							newTypesValue = irEmitter.emitInsertValue(newTypesValue,
							                                          templateRefEntryValue,
							                                          { (unsigned int) i });
						} else {
							// Other values are just ignored for now...
							llvm::Value* typeInfo = constGen.getUndef(typeInfoType(module).second);
							typeInfo = irEmitter.emitInsertValue(typeInfo,
							                                     nullTemplateGenerator(module),
							                                     { 1 });
							newTypesValue = irEmitter.emitInsertValue(newTypesValue,
							                                          typeInfo,
							                                          { (unsigned int) i });
						}
						continue;
					}
					
					const auto& templateUseArg = templateUseInst.arguments()[i].typeRefType();
					if (templateUseArg->isTemplateVar()) {
						// For template variables, just copy across the existing type
						// from the types provided to us by the caller.
						const auto templateVarIndex = templateUseArg->getTemplateVar()->index();
						const auto templateVarValue = builder.CreateExtractValue(typesArg, templateVarIndex);
						newTypesValue = irEmitter.emitInsertValue(newTypesValue, templateVarValue, { (unsigned int) i });
					} else {
						// For an object type need to obtain the vtable (and potentially
						// also the generator function for its template arguments).
						const auto vtablePointer = genVTable(module, templateUseArg->resolveAliases()->getObjectType());
						
						llvm::Value* typeInfo = constGen.getUndef(typeInfoType(module).second);
						typeInfo = irEmitter.emitInsertValue(typeInfo, vtablePointer, { 0 });
						
						if (templateUseArg->templateArguments().empty()) {
							// If the type doesn't have any template arguments, then
							// provide a 'null' template generator.
							typeInfo = irEmitter.emitInsertValue(typeInfo, nullTemplateGenerator(module), { 1 });
						} else {
							// If there are arguments, refer to the component for them
							// by adding the correct component in the path by computing
							// (subPath & ~mask) | <their component>.
							const auto argComponent = templateBuilder.templateUseMap().at(TemplateInst::Type(templateUseArg));
							const auto maskedSubPath = builder.CreateAnd(subPath, builder.CreateNot(mask));
							const auto argFullPath = builder.CreateOr(maskedSubPath, constGen.getI32(argComponent));
							
							llvm::Value* templateGenerator = constGen.getUndef(templateGeneratorType(module).second);
							templateGenerator = irEmitter.emitInsertValue(templateGenerator, rootFnArg, { 0 });
							templateGenerator = irEmitter.emitInsertValue(templateGenerator, rootContextArg, { 1 });
							templateGenerator = irEmitter.emitInsertValue(templateGenerator, argFullPath, { 2 });
							typeInfo = irEmitter.emitInsertValue(typeInfo, templateGenerator, { 1 });
						}
						
						newTypesValue = irEmitter.emitInsertValue(newTypesValue, typeInfo, { (unsigned int) i });
					}
				}
				
				if (templateUseInst.object().isTypeInstance() && templateUseInst.object().typeInstance()->isPrimitive()) {
					// Primitives don't have template generators;
					// the path must have ended by now.
					irEmitter.emitReturn(typeInfoArrayType(module).second,
					                     newTypesValue);
				} else {
					// If the position is 0, that means we've reached the end of the path,
					// so just return the new types struct. Otherwise, call the next
					// intermediate template generator.
					const auto pathEndBB = function.createBasicBlock("pathEnd");
					const auto callNextGeneratorBB = function.createBasicBlock("callNextGenerator");
					
					const auto compareValue = builder.CreateICmpEQ(position, constGen.getI8(0));
					builder.CreateCondBr(compareValue, pathEndBB, callNextGeneratorBB);
					
					function.selectBasicBlock(pathEndBB);
					irEmitter.emitReturn(typeInfoArrayType(module).second,
					                     newTypesValue);
					
					function.selectBasicBlock(callNextGeneratorBB);
					
					// Call the next intermediate function.
					const auto nextFunction = genTemplateIntermediateFunctionDecl(module, templateUseInst.object());
					
					llvm::Value* const args[] = { newTypesValue, rootFnArg, rootContextArg, pathArg, position };
					const auto callResult = genRawFunctionCall(function, argInfo, nextFunction, args);
					irEmitter.emitReturn(typeInfoArrayType(module).second,
					                     callResult);
				}
			}
			
			function.selectBasicBlock(endBB);
			
			// Path is not valid...
			builder.CreateUnreachable();
			
			return llvmFunction;
		}
		
	}
	
}

