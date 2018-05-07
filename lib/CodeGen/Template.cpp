#include <locic/CodeGen/Template.hpp>

#include <array>
#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include <llvm-abi/ABITypeInfo.hpp>

#include <locic/AST/Function.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ASTFunctionGenerator.hpp>
#include <locic/CodeGen/CallEmitter.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenDebugType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/ValueEmitter.hpp>

namespace locic {

	namespace CodeGen {
		
		bool isRootArgumentList(llvm::ArrayRef<AST::Value> templateArguments);
		
		bool isRootType(const AST::Type* const type) {
			switch (type->kind()) {
				case AST::Type::OBJECT: {
					// Interface type template arguments don't affect
					// code generation in any way so they can be
					// ignored here.
					return type->getObjectType()->isInterface() ||
					       isRootArgumentList(arrayRef(type->templateArguments()));
				}
				case AST::Type::TEMPLATEVAR: {
					return false;
				}
				case AST::Type::ALIAS: {
					return isRootType(type->resolveAliases());
				}
				default: {
					llvm_unreachable("Unknown AST::Type kind in isRootType()");
				}
			}
		}
		
		bool isRootArgument(const AST::Value& value) {
			if (value.isTypeRef()) {
				return isRootType(value.typeRefType());
			} else if (value.isTemplateVarRef()) {
				return false;
			} else {
				return true;
			}
		}
		
		bool isRootArgumentList(llvm::ArrayRef<AST::Value> templateArguments) {
			for (size_t i = 0; i < templateArguments.size(); i++) {
				if (!isRootArgument(templateArguments[i])) {
					return false;
				}
			}
			return true;
		}
		
		constexpr size_t TYPE_INFO_ARRAY_SIZE = 8;
		
		llvm_abi::Type pathType(Module& /*module*/) {
			return llvm_abi::Int64Ty;
		}
		
		llvm_abi::Type templateGeneratorABIType(Module& module) {
			auto& abiTypeBuilder = module.abiTypeBuilder();
			llvm::SmallVector<llvm_abi::Type, 2> types;
			types.push_back(llvm_abi::PointerTy);
			types.push_back(llvm_abi::Int64Ty);
			return llvm_abi::Type::AutoStruct(abiTypeBuilder, types, "__template_generator");
		}
		
		llvm_abi::Type templateGeneratorType(Module& module) {
			const auto iterator = module.standardTypeMap().find(TemplateGeneratorType);
			if (iterator != module.standardTypeMap().end()) {
				return iterator->second;
			}
			
			const auto type = templateGeneratorABIType(module);
			module.standardTypeMap().insert(std::make_pair(TemplateGeneratorType, type));
			return type;
		}
		
		llvm_abi::Type typeInfoABIType(Module& module) {
			auto& abiTypeBuilder = module.abiTypeBuilder();
			llvm::SmallVector<llvm_abi::Type, 2> types;
			types.push_back(llvm_abi::PointerTy);
			types.push_back(templateGeneratorType(module));
			return llvm_abi::Type::AutoStruct(abiTypeBuilder, types, "__type_info");
		}
		
		llvm_abi::Type typeInfoType(Module& module) {
			const auto iterator = module.standardTypeMap().find(TypeInfoType);
			if (iterator != module.standardTypeMap().end()) {
				return iterator->second;
			}
			
			const auto type = typeInfoABIType(module);
			module.standardTypeMap().insert(std::make_pair(TypeInfoType, type));
			return type;
		}
		
		llvm_abi::Type typeInfoArrayType(Module& module) {
			const auto typeInfo = typeInfoType(module);
			auto& abiTypeBuilder = module.abiTypeBuilder();
			return llvm_abi::Type::Array(abiTypeBuilder, TYPE_INFO_ARRAY_SIZE, typeInfo);
		}
		
		ArgInfo rootFunctionArgInfo(Module& module) {
			llvm::SmallVector<llvm_abi::Type, 2> types;
			
			// Type info array pointer.
			types.push_back(llvm_abi::PointerTy);
			
			// Path value.
			types.push_back(pathType(module));
			
			return ArgInfo::VoidBasic(module, types).withNoExcept();
		}
		
		llvm::Value* computeTemplateArguments(Function& function, llvm::Value* generatorValue) {
			auto& builder = function.getBuilder();
			auto& module = function.module();
			
			IREmitter irEmitter(function);
			const auto arrayType = typeInfoArrayType(module);
			const auto typesPtrArg = irEmitter.emitRawAlloca(arrayType);
			
			const auto generatorRootFn = builder.CreateExtractValue(generatorValue, { 0 }, "rootFn");
			const auto generatorPath = builder.CreateExtractValue(generatorValue, { 1 }, "path");
			
			const auto argInfo = rootFunctionArgInfo(function.module());
			llvm::Value* const args[] = { typesPtrArg, generatorPath };
			
			CallEmitter callEmitter(irEmitter);
			callEmitter.emitRawCall(argInfo, generatorRootFn, args);
			
			const auto result = irEmitter.emitRawLoad(typesPtrArg, arrayType);
			result->setName("templateArgs");
			return result;
		}
		
		llvm::Constant* nullTemplateGenerator(Module& module) {
			ConstantGenerator constGen(module);
			
			llvm::Constant* const values[] = {
				constGen.getNullPointer(),
				constGen.getI64(0)
			};
			
			const auto type = module.getLLVMType(templateGeneratorType(module));
			return constGen.getStruct(static_cast<llvm::StructType*>(type), values);
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
						constGen.getI64(1)
					};
				const auto type = module.getLLVMType(templateGeneratorType(module));
				return constGen.getStruct(static_cast<llvm::StructType*>(type), values);
			} else {
				auto& templateBuilder = function.templateBuilder();
				
				// Add a use to the parent type's intermediate template generator.
				const auto entryId = templateBuilder.addUse(templateInst);
				
				const auto parentTemplateGenerator = function.getTemplateGenerator();
				if (entryId == (size_t) -1) {
					return parentTemplateGenerator;
				}
				
				const auto rootFunction = builder.CreateExtractValue(parentTemplateGenerator, { 0 });
				const auto path = builder.CreateExtractValue(parentTemplateGenerator, { 1 });
				
				// Insert garbage value for bits required; this will be replaced
				// later by the template builder when the actual number of bits
				// required is known.
				const auto bitsRequiredGarbageValue = constGen.getI64(4242);
				
				const auto shiftedPath = builder.CreateShl(path, bitsRequiredGarbageValue);
				
				// Record the instruction so it can be modified later when we
				// actually know how many intermediate template uses exist (and
				// hence know the bits required).
				templateBuilder.addInstruction(llvm::dyn_cast<llvm::Instruction>(shiftedPath));
				
				const auto newPath = builder.CreateOr(shiftedPath, constGen.getI64(entryId));
				
				SetUseEntryBuilder setUseEntryBuilder(function);
				
				auto templateGenerator = irEmitter.getUndef(templateGeneratorType(module));
				templateGenerator = irEmitter.emitInsertValue(templateGenerator, rootFunction, { 0 });
				templateGenerator = irEmitter.emitInsertValue(templateGenerator, newPath, { 1 });
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
		
		llvm::Function* genTemplateValueFunction(Function& parentFunction, const AST::Value& value) {
			auto& module = parentFunction.module();
			
			AST::FunctionAttributes attributes(/*isVarArg=*/false,
			                                   /*isMethod=*/false,
			                                   /*isTemplated=*/false,
			                                   /*noExceptPredicate=*/AST::Predicate::True());
			AST::FunctionType functionType(std::move(attributes),
			                               /*returnType=*/value.type(),
			                               /*parameterTypes=*/{});
			
			const auto argInfo = ArgInfo::FromAST(module, functionType);
			const auto llvmFunction = argInfo.createFunction("template_value",
			                                                 llvm::Function::InternalLinkage);
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			Function functionGenerator(module, *llvmFunction, argInfo);
			
			const auto parentInfo = parentFunction.debugInfo();
			
			const auto debugFunctionType = genDebugFunctionType(module, functionType);
			const auto debugInfo = module.debugBuilder().createFunction(/*scope=*/parentInfo,
			                                            value.debugInfo() ? value.debugInfo()->location.range().start().lineNumber() : 0,
			                                            /*isInternal=*/true,
			                                            /*isDefinition=*/true,
			                                            Name::Relative(),
			                                            debugFunctionType,
			                                            llvmFunction);
			functionGenerator.attachDebugInfo(debugInfo);
			
			if (value.debugInfo()) {
				functionGenerator.setDebugPosition(value.debugInfo()->location.range().start());
			}
			
			IREmitter irEmitter(functionGenerator);
			ValueEmitter valueEmitter(irEmitter);
			
			const auto result = valueEmitter.emitValue(value,
			                                           functionGenerator.getReturnVarOrNull());
			irEmitter.emitReturn(result);
			
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
			const auto llvmFunction = argInfo.createFunction("template_root",
			                                                 llvm::Function::InternalLinkage);
			
			module.templateRootFunctionMap().insert(std::make_pair(templateInst.copy(), llvmFunction));
			
			// Always inline template generators.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			Function function(module, *llvmFunction, argInfo);
			IREmitter irEmitter(function);
			
			auto& builder = function.getBuilder();
			
			const auto typesPtrArg = function.getArg(0);
			const auto pathArg = function.getArg(1);
			
			const auto arrayType = typeInfoArrayType(module);
			
			ConstantGenerator constGen(module);
			
			for (size_t i = 0; i < templateInst.arguments().size(); i++) {
				const auto& templateArg = templateInst.arguments()[i];
				
				auto typeInfo = irEmitter.getUndef(typeInfoType(module));
				
				if (templateArg.isTypeRef()) {
					const auto vtablePointer = genVTable(module, templateArg.typeRefType()->resolveAliases()->getObjectType());
					typeInfo = irEmitter.emitInsertValue(typeInfo, vtablePointer, { 0 });
					
					const auto generator = getTemplateGenerator(function, TemplateInst::Type(templateArg.typeRefType()));
					typeInfo = irEmitter.emitInsertValue(typeInfo, generator, { 1 });
				} else {
					const auto valueFunction = genTemplateValueFunction(parentFunction, templateArg);
					typeInfo = irEmitter.emitInsertValue(typeInfo, valueFunction, { 0 });
					
					const auto generator = nullTemplateGenerator(module);
					typeInfo = irEmitter.emitInsertValue(typeInfo, generator, { 1 });
				}
				
				const auto typeInfoGEP = irEmitter.emitConstInBoundsGEP2_32(arrayType,
				                                                            typesPtrArg,
				                                                            0, i);
				irEmitter.emitRawStore(typeInfo, typeInfoGEP);
			}
			
			if (templateInst.object().isTypeInstance() && templateInst.object().typeInstance()->isPrimitive()) {
				// Primitives don't have template generators;
				// the path must have ended by now.
				irEmitter.emitReturnVoid();
				return llvmFunction;
			}
			
			TypeGenerator typeGen(module);
			
			llvm::Type* const ctlzTypes[] = { typeGen.getI64Type() };
			const auto countLeadingZerosFunction = llvm::Intrinsic::getDeclaration(function.module().getLLVMModulePtr(), llvm::Intrinsic::ctlz, ctlzTypes);
			llvm::Value* const ctlzArgs[] = { pathArg, constGen.getI1(true) };
			const auto numLeadingZeroes = irEmitter.emitCall(countLeadingZerosFunction->getFunctionType(),
			                                                 countLeadingZerosFunction, ctlzArgs);
			const auto numLeadingZeroesSize = builder.CreateZExtOrTrunc(numLeadingZeroes, typeGen.getSizeTType());
			const auto startPosition = builder.CreateSub(constGen.getSizeTValue(63), numLeadingZeroesSize);
			
			const auto nextFunction = genTemplateIntermediateFunctionDecl(module, templateInst.object());
			
			llvm::Value* const args[] = { typesPtrArg, llvmFunction, pathArg, startPosition };
			
			CallEmitter callEmitter(irEmitter);
			callEmitter.emitRawCall(intermediateFunctionArgInfo(module), nextFunction, args);
			
			irEmitter.emitReturnVoid();
			return llvmFunction;
		}
		
		ArgInfo intermediateFunctionArgInfo(Module& module) {
			std::vector<llvm_abi::Type> argTypes;
			argTypes.reserve(4);
			
			// Type info array pointer.
			argTypes.push_back(llvm_abi::PointerTy);
			
			// Root function pointer.
			argTypes.push_back(llvm_abi::PointerTy);
			
			// Path value.
			argTypes.push_back(pathType(module));
			
			// Position in path.
			argTypes.push_back(llvm_abi::SizeTy);
			
			return ArgInfo::VoidBasic(module, argTypes).withNoExcept();
		}
		
		static llvm::GlobalValue::LinkageTypes
		getTemplatedObjectLinkage(Module& module,
		                          TemplatedObject templatedObject) {
			auto& astFunctionGenerator = module.astFunctionGenerator();
			if (templatedObject.isTypeInstance()) {
				return astFunctionGenerator.getTypeLinkage(*(templatedObject.typeInstance()));
			} else {
				return astFunctionGenerator.getLinkage(templatedObject.parentTypeInstance(),
				                                       *(templatedObject.function()));
			}
		}
		
		static bool isTemplatedObjectDecl(Module& module, TemplatedObject templatedObject) {
			auto& astFunctionGenerator = module.astFunctionGenerator();
			if (templatedObject.isTypeInstance()) {
				return templatedObject.typeInstance()->isClassDecl();
			} else {
				return !astFunctionGenerator.hasDef(templatedObject.parentTypeInstance(),
				                                    *(templatedObject.function()));
			}
		}
		
		llvm::Function* genTemplateIntermediateFunctionDecl(Module& module, TemplatedObject templatedObject) {
			assert(!templatedObject.isTypeInstance() || !templatedObject.typeInstance()->isPrimitive());
			const auto mangledName = mangleTemplateGenerator(module, templatedObject);
			
			const auto iterator = module.getFunctionMap().find(mangledName);
			if (iterator != module.getFunctionMap().end()) {
				return iterator->second;
			}
			
			const auto argInfo = intermediateFunctionArgInfo(module);
			const auto llvmFunction =
				argInfo.createFunction(mangledName.c_str(),
				                       getTemplatedObjectLinkage(module, templatedObject));
			module.getFunctionMap().insert(std::make_pair(mangledName, llvmFunction));
			return llvmFunction;
		}
		
		class TypeArrayMapper {
		public:
			TypeArrayMapper() { }
			
			void scheduleAssign(const size_t index, llvm::Value* const value) {
				SetAction action;
				action.index = index;
				action.value = value;
				setActions_.push_back(action);
			}
			
			void scheduleMove(const size_t sourceIndex, const size_t destIndex) {
				if (sourceIndex == destIndex) return;
				
				MoveAction action;
				action.sourceIndex = sourceIndex;
				action.destIndex = destIndex;
				moveActions_.push_back(action);
			}
			
			void emitActions(IREmitter& irEmitter, llvm::Value* const typesPtrArg) {
				const auto type = typeInfoType(irEmitter.module());
				const auto arrayType = typeInfoArrayType(irEmitter.module());
				
				llvm::SmallVector<llvm::Value*, 8> loadedValues;
				loadedValues.reserve(moveActions_.size());
				
				// First load old values.
				for (const auto& moveAction: moveActions_) {
					const auto loadGEP = irEmitter.emitConstInBoundsGEP2_32(arrayType,
					                                                        typesPtrArg,
					                                                        0, moveAction.sourceIndex);
					loadedValues.push_back(irEmitter.emitRawLoad(loadGEP, type));
				}
				
				// Then store the new values.
				for (size_t i = 0; i < moveActions_.size(); i++) {
					const auto& moveAction = moveActions_[i];
					const auto storeGEP = irEmitter.emitConstInBoundsGEP2_32(arrayType,
					                                                         typesPtrArg,
					                                                         0, moveAction.destIndex);
					irEmitter.emitRawStore(loadedValues[i], storeGEP);
				}
				
				for (const auto& setAction: setActions_) {
					const auto storeGEP = irEmitter.emitConstInBoundsGEP2_32(arrayType,
					                                                         typesPtrArg,
					                                                         0, setAction.index);
					irEmitter.emitRawStore(setAction.value, storeGEP);
				}
			}
			
		private:
			struct MoveAction {
				size_t sourceIndex;
				size_t destIndex;
			};
			
			llvm::SmallVector<MoveAction, 8> moveActions_;
			
			struct SetAction {
				size_t index;
				llvm::Value* value;
			};
			
			llvm::SmallVector<SetAction, 8> setActions_;
			
		};
		
		llvm::Function* genTemplateIntermediateFunction(Module& module, TemplatedObject templatedObject, const TemplateBuilder& templateBuilder) {
			ConstantGenerator constGen(module);
			
			const auto argInfo = intermediateFunctionArgInfo(module);
			const auto llvmFunction = genTemplateIntermediateFunctionDecl(module, templatedObject);
			
			if (isTemplatedObjectDecl(module, templatedObject)) {
				return llvmFunction;
			}
			
			Function function(module, *llvmFunction, argInfo);
			
			// Always inline template generators.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			const auto typesPtrArg = function.getArg(0);
			const auto rootFnArg = function.getArg(1);
			const auto pathArg = function.getArg(2);
			const auto parentPositionArg = function.getArg(3);
			
			IREmitter irEmitter(function);
			
			if (templateBuilder.templateUseMap().empty()) {
				irEmitter.emitReturnVoid();
				return llvmFunction;
			}
			
			auto& builder = function.getBuilder();
			
			// If the position is 0 that means that the path terminates
			// here so just return the types array.
			// Otherwise continue to the next intermediate template generator.
			const auto pathEndBB = irEmitter.createBasicBlock("pathEnd");
			const auto callNextGeneratorBB = irEmitter.createBasicBlock("callNextGenerator");
			
			const auto compareValue = builder.CreateICmpEQ(parentPositionArg, constGen.getSizeTValue(0));
			irEmitter.emitCondBranch(compareValue, pathEndBB, callNextGeneratorBB);
			
			irEmitter.selectBasicBlock(pathEndBB);
			irEmitter.emitReturnVoid();
			
			irEmitter.selectBasicBlock(callNextGeneratorBB);
			
			const auto bitsRequiredValue = constGen.getSizeTValue(templateBuilder.bitsRequired());
			const auto position = builder.CreateSub(parentPositionArg, bitsRequiredValue);
			const auto castPosition = builder.CreateZExtOrTrunc(position, TypeGenerator(module).getI64Type());
			const auto subPath = builder.CreateLShr(pathArg, castPosition);
			const auto mask = constGen.getI64((1 << templateBuilder.bitsRequired()) - 1);
			const auto component = builder.CreateAnd(subPath, mask);
			
			const auto endBB = irEmitter.createBasicBlock("");
			const auto switchInstruction = builder.CreateSwitch(component, endBB);
			
			// Generate the component entry for each template use.
			for (const auto& templateUsePair: templateBuilder.templateUseMap()) {
				const auto& templateUseInst = templateUsePair.first;
				const auto templateUseComponent = templateUsePair.second;
				
				const auto caseBB = irEmitter.createBasicBlock("");
				switchInstruction->addCase(constGen.getI64(templateUseComponent), caseBB);
				
				irEmitter.selectBasicBlock(caseBB);
				
				TypeArrayMapper typeArrayMapper;
				
				// Loop through each template argument and generate it.
				for (size_t i = 0; i < templateUseInst.arguments().size(); i++) {
					if (!templateUseInst.arguments()[i].isTypeRef()) {
						if (templateUseInst.arguments()[i].isTemplateVarRef()) {
							// Template var ref values just need to
							// propagate the relevant argument.
							const auto templateUseVar = templateUseInst.arguments()[i].templateVar();
							const auto templateVarIndex = templateUseVar->index();
							typeArrayMapper.scheduleMove(templateVarIndex, i);
						} else {
							// Other values are just ignored for now...
							auto typeInfo = irEmitter.getUndef(typeInfoType(module));
							typeInfo = irEmitter.emitInsertValue(typeInfo,
							                                     nullTemplateGenerator(module),
							                                     { 1 });
							typeArrayMapper.scheduleAssign(i, typeInfo);
						}
						continue;
					}
					
					const auto& templateUseArg = templateUseInst.arguments()[i].typeRefType();
					if (templateUseArg->isTemplateVar()) {
						// For template variables, just copy across the existing type
						// from the types provided to us by the caller.
						const auto templateVarIndex = templateUseArg->getTemplateVar()->index();
						typeArrayMapper.scheduleMove(templateVarIndex, i);
					} else {
						// For an object type need to obtain the vtable (and potentially
						// also the generator function for its template arguments).
						const auto vtablePointer = genVTable(module, templateUseArg->resolveAliases()->getObjectType());
						
						llvm::Value* typeInfo = irEmitter.getUndef(typeInfoType(module));
						typeInfo = irEmitter.emitInsertValue(typeInfo, vtablePointer, { 0 });
						
						if (templateUseArg->templateArguments().empty()) {
							// If the type doesn't have any template arguments, then
							// provide a 'null' template generator.
							typeInfo = irEmitter.emitInsertValue(typeInfo, nullTemplateGenerator(module), { 1 });
						} else {
							// If there are arguments, we need to create a template
							// generator for them.
							const auto argComponent = templateBuilder.getUse(TemplateInst::Type(templateUseArg));
							assert(argComponent);
							
							llvm::Value* argFullPath;
							
							if (argComponent.value() == (size_t) -1) {
								// Reference to self with the same template
								// args, so use the parent path.
								argFullPath = builder.CreateLShr(subPath, constGen.getI64(templateBuilder.bitsRequired()));
							} else {
								// Create a template generator for the arguments
								// by adding the correct component in the path by
								// computing (subPath & ~mask) | <their component>.
								const auto maskedSubPath = builder.CreateAnd(subPath, builder.CreateNot(mask));
								argFullPath = builder.CreateOr(maskedSubPath, constGen.getI64(argComponent.value()));
							}
							
							auto templateGenerator = irEmitter.getUndef(templateGeneratorType(module));
							templateGenerator = irEmitter.emitInsertValue(templateGenerator, rootFnArg, { 0 });
							templateGenerator = irEmitter.emitInsertValue(templateGenerator, argFullPath, { 1 });
							typeInfo = irEmitter.emitInsertValue(typeInfo, templateGenerator, { 1 });
						}
						
						typeArrayMapper.scheduleAssign(i, typeInfo);
					}
				}
				
				typeArrayMapper.emitActions(irEmitter, typesPtrArg);
				
				if (templateUseInst.object().isTypeInstance() && templateUseInst.object().typeInstance()->isPrimitive()) {
					// Primitives don't have template generators;
					// the path must have ended by now.
					irEmitter.emitReturnVoid();
				} else {
					// Call the next intermediate function.
					const auto nextFunction = genTemplateIntermediateFunctionDecl(module, templateUseInst.object());
					
					llvm::Value* const args[] = { typesPtrArg, rootFnArg, pathArg, position };
					
					CallEmitter callEmitter(irEmitter);
					callEmitter.emitRawCall(argInfo, nextFunction, args);
					
					irEmitter.emitReturnVoid();
				}
			}
			
			irEmitter.selectBasicBlock(endBB);
			
			// Path is not valid...
			irEmitter.emitUnreachable();
			
			return llvmFunction;
		}
		
	}
	
}

