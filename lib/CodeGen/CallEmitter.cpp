#include <locic/CodeGen/CallEmitter.hpp>

#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>

#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/TemplateVar.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Exception.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/MethodInfo.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/ValueEmitter.hpp>
#include <locic/CodeGen/VirtualCallABI.hpp>
#include <locic/CodeGen/VTable.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace CodeGen {
		
		CallEmitter::CallEmitter(IREmitter& irEmitter)
		: irEmitter_(irEmitter) { }
		
		llvm::Value*
		CallEmitter::emitASTCall(const AST::Value& astCallValue,
		                         llvm::ArrayRef<AST::Value> args,
		                         llvm::Value* const resultPtr) {
			ValueEmitter valueEmitter(irEmitter_);
			
			const auto functionType = astCallValue.type()->asFunctionType();
			
			llvm::SmallVector<llvm_abi::TypedValue, 10> parameters;
			parameters.reserve(args.size());
			
			// Make sure to evaluate arguments first.
			for (const auto& param: args) {
				const auto paramType = param.type();
				const auto abiType = genABIArgType(irEmitter_.module(), paramType);
				const auto irValue = valueEmitter.emitValue(param);
				parameters.push_back(llvm_abi::TypedValue(irValue,
				                                          abiType));
			}
			
			const auto callInfo = genFunctionCallInfo(irEmitter_.function(),
			                                          astCallValue);
			return emitCall(functionType, callInfo, parameters,
			                resultPtr);
		}
		
		llvm::Value*
		CallEmitter::emitRawCall(const ArgInfo& argInfo,
		                         llvm::Value* functionPtr,
		                         llvm::ArrayRef<llvm::Value*> args,
		                         const bool musttail) {
			assert((!argInfo.isVarArg() || musttail) && "This method doesn't support calling varargs functions.");
			const auto functionABIType = argInfo.getABIFunctionType();
			llvm::SmallVector<llvm_abi::TypedValue, 10> abiArgs;
			abiArgs.reserve(args.size());
			for (size_t i = 0; i < args.size(); i++) {
				abiArgs.push_back(llvm_abi::TypedValue(args[i],
				                                       functionABIType.argumentTypes()[i]));
			}
			return emitRawCall(argInfo, functionPtr,
			                   abiArgs, musttail);
		}
		
		llvm::Value*
		CallEmitter::emitRawCall(const ArgInfo& argInfo,
		                         llvm::Value* functionPtr,
		                         llvm::ArrayRef<llvm_abi::TypedValue> args,
		                         const bool musttail) {
			if (argInfo.isVarArg()) {
				assert(args.size() >= argInfo.numArguments());
			} else {
				assert(args.size() == argInfo.numArguments());
			}
			
			const auto functionABIType = argInfo.getABIFunctionType();
			const auto functionIRType = argInfo.makeFunctionType();
			
			auto& module = irEmitter_.module();
			
			const auto callBuilder = [&](llvm::ArrayRef<llvm::Value*> encodedArgs) -> llvm::Value* {
				// If the function can throw AND we have pending unwind actions,
				// emit a landing pad to execute those actions.
				if (!argInfo.noExcept() && anyUnwindActions(irEmitter_.function(), UnwindStateThrow)) {
					assert(!musttail);
					
					const auto successPath = irEmitter_.createBasicBlock("");
					const auto failPath = genLandingPad(irEmitter_.function(), UnwindStateThrow);
					
					const auto invokeInst = irEmitter_.emitInvoke(functionIRType,
					                                              functionPtr,
					                                              successPath,
					                                              failPath,
					                                              encodedArgs);
					
					if (argInfo.noReturn()) {
						invokeInst->setDoesNotReturn();
					}
					
					if (argInfo.noMemoryAccess()) {
						invokeInst->setDoesNotAccessMemory();
					}
					
					if (argInfo.hasNestArgument()) {
						invokeInst->addAttribute(1, llvm::Attribute::Nest);
					}
					
					const auto attributes = module.abi().getAttributes(functionABIType,
					                                                   functionABIType.argumentTypes(),
					                                                   invokeInst->getAttributes());
					invokeInst->setAttributes(attributes);
					
					irEmitter_.selectBasicBlock(successPath);
					return invokeInst;
				} else {
					const auto callInst = irEmitter_.emitCall(functionIRType,
					                                          functionPtr,
					                                          encodedArgs);
					if (argInfo.noExcept()) {
						callInst->setDoesNotThrow();
					}
					
					if (argInfo.noReturn()) {
						callInst->setDoesNotReturn();
					}
					
					if (argInfo.noMemoryAccess()) {
						callInst->setDoesNotAccessMemory();
					}
					
#if LOCIC_LLVM_VERSION >= 305
					if (musttail) {
						callInst->setTailCallKind(llvm::CallInst::TCK_MustTail);
					}
#else
					assert(!musttail && "musttail not supported with this version of LLVM");
#endif
					
					if (argInfo.hasNestArgument()) {
						callInst->addAttribute(1, llvm::Attribute::Nest);
					}
					
					const auto attributes = module.abi().getAttributes(functionABIType,
					                                                   functionABIType.argumentTypes(),
					                                                   callInst->getAttributes());
					callInst->setAttributes(attributes);
					
					return callInst;
				}
			};
			
			return module.abi().createCall(irEmitter_.function(),
			                               functionABIType,
			                               callBuilder,
			                               args);
		}
		
		llvm::Value*
		CallEmitter::emitNonVarArgsCall(AST::FunctionType functionType,
		                                const FunctionCallInfo callInfo,
		                                PendingResultArray args,
		                                llvm::Value* const resultPtr) {
			const auto argInfo = ArgInfo::FromAST(irEmitter_.module(), functionType);
			assert(!argInfo.isVarArg() && "This method doesn't support calling varargs functions.");
			assert(args.size() == argInfo.numStandardArguments());
			
			const auto functionABIType = argInfo.getABIFunctionType();
			llvm::SmallVector<llvm_abi::TypedValue, 10> abiArgs;
			abiArgs.reserve(args.size());
			for (size_t i = 0; i < args.size(); i++) {
				const auto abiTypeIndex = argInfo.standardArgumentOffset() + i;
				abiArgs.push_back(llvm_abi::TypedValue(args[i].resolve(irEmitter_.function()),
				                                       functionABIType.argumentTypes()[abiTypeIndex]));
			}
			return emitCall(functionType, callInfo, abiArgs, resultPtr);
		}
		
		llvm::Value*
		CallEmitter::emitCall(AST::FunctionType functionType,
		                      const FunctionCallInfo callInfo,
		                      llvm::ArrayRef<llvm_abi::TypedValue> args,
		                      llvm::Value* const resultPtr) {
			auto& module = irEmitter_.module();
			
			const auto argInfo = ArgInfo::FromAST(module, functionType);
			
			llvm::SmallVector<llvm_abi::TypedValue, 10> llvmArgs;
			llvmArgs.reserve(3 + args.size());
			
			// Some values (e.g. classes) will be returned
			// by assigning to a pointer passed as the first
			// argument (this deals with the class sizes
			// potentially being unknown).
			llvm::Value* returnVar = nullptr;
			if (argInfo.hasReturnVarArgument()) {
				returnVar = irEmitter_.emitAlloca(functionType.returnType(), resultPtr);
				llvmArgs.push_back(llvm_abi::TypedValue(returnVar,
				                                        llvm_abi::PointerTy));
			}
			
			if (argInfo.isVarArg() && argInfo.hasTemplateGeneratorArgument()) {
				assert(callInfo.templateGenerator != nullptr);
				llvmArgs.push_back(llvm_abi::TypedValue(callInfo.templateGenerator,
				                                        templateGeneratorType(module)));
			}
			
			if (argInfo.hasContextArgument()) {
				assert(callInfo.contextPointer != nullptr);
				llvmArgs.push_back(llvm_abi::TypedValue(callInfo.contextPointer,
				                                        llvm_abi::PointerTy));
			}
			
			for (const auto& arg: args) {
				llvmArgs.push_back(arg);
			}
			
			if (!argInfo.isVarArg() && argInfo.hasTemplateGeneratorArgument()) {
				assert(callInfo.templateGenerator != nullptr);
				llvmArgs.push_back(llvm_abi::TypedValue(callInfo.templateGenerator,
				                                        templateGeneratorType(module)));
			}
			
			const auto result = emitRawCall(argInfo, callInfo.functionPtr,
			                                llvmArgs);
			
			if (argInfo.hasReturnVarArgument()) {
				// As above, if the return value pointer is used,
				// this should be loaded (and used instead).
				return irEmitter_.emitLoad(returnVar, functionType.returnType());
			} else {
				return result;
			}
		}
		
		llvm::Value*
		CallEmitter::emitTemplateMethodCall(const MethodInfo& methodInfo,
		                                    Optional<PendingResult> methodOwner,
		                                    PendingResultArray args,
		                                    llvm::Value* const resultPtr) {
			const auto parentType = methodInfo.parentType;
			assert(parentType->isTemplateVar());
			
			const auto templateVar = parentType->getTemplateVar();
			
			auto& function = irEmitter_.function();
			auto& module = function.module();
			
			const auto typeInfo = function.getBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) templateVar->index() });
			
			const auto methodHash = CreateMethodNameHash(methodInfo.name);
			const auto methodHashValue = ConstantGenerator(module).getI64(methodHash);
			
			const auto contextPointer = methodOwner ?
			                            methodOwner->resolve(function) :
			                            ConstantGenerator(module).getNull(TypeGenerator(module).getPtrType());
			
			llvm::SmallVector<llvm::Value*, 10> llvmArgs;
			llvmArgs.reserve(args.size());
			for (auto& arg: args) {
				llvmArgs.push_back(arg.resolve(function));
			}
			
			const VirtualObjectComponents objectComponents(getTypeInfoComponents(function, typeInfo), contextPointer);
			const VirtualMethodComponents methodComponents(objectComponents, methodHashValue);
			
			return module.virtualCallABI().emitCall(irEmitter_,
			                                        methodInfo.functionType,
			                                        methodComponents,
			                                        llvmArgs,
			                                        resultPtr);
		}
		
		llvm::Value*
		CallEmitter::emitMethodCall(const MethodInfo& methodInfo,
		                            Optional<PendingResult> methodOwner,
		                            PendingResultArray args,
		                            llvm::Value* const resultPtr) {
			auto& function = irEmitter_.function();
			
			const auto type = methodInfo.parentType;
			
			assert(type != nullptr);
			assert(type->isObjectOrTemplateVar());
			
			if (type->isObject()) {
				if (type->isPrimitive()) {
					PendingResultArray newArgs;
					if (methodOwner) {
						newArgs.push_back(std::move(*methodOwner));
					}
					
					for (auto& arg: args) {
						newArgs.push_back(std::move(arg));
					}
					
					return genTrivialPrimitiveFunctionCall(function, methodInfo, std::move(newArgs), resultPtr);
				} else {
					FunctionCallInfo callInfo;
					
					const auto& targetFunction = type->getObjectType()->getFunction(CanonicalizeMethodName(methodInfo.name));
					callInfo.functionPtr = genFunctionRef(function, type, &targetFunction, methodInfo.functionType);
					
					if (!type->getObjectType()->templateVariables().empty()) {
						callInfo.templateGenerator = getTemplateGenerator(function, TemplateInst::Type(type));
					}
					
					if (methodOwner) {
						callInfo.contextPointer = methodOwner->resolve(function);
					}
					
					return emitNonVarArgsCall(methodInfo.functionType,
					                          callInfo, std::move(args),
					                          resultPtr);
				}
			} else {
				return emitTemplateMethodCall(methodInfo, std::move(methodOwner),
				                              std::move(args), resultPtr);
			}
		}
		
		llvm::Value*
		CallEmitter::emitDynamicMethodCall(const MethodInfo& methodInfo,
		                                   PendingResult methodOwner,
		                                   PendingResultArray args,
		                                   llvm::Value* const resultPtr) {
			return emitMethodCall(methodInfo, Optional<PendingResult>(std::move(methodOwner)),
			                      std::move(args), resultPtr);
		}
		
		llvm::Value*
		CallEmitter::emitStaticMethodCall(const MethodInfo& methodInfo,
		                                  PendingResultArray args,
		                                  llvm::Value* const resultPtr) {
			return emitMethodCall(methodInfo, None, std::move(args), resultPtr);
		}
		
	}
	
}

