#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Exception.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/MethodInfo.hpp>
#include <locic/CodeGen/Move.hpp>
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
		
		llvm::Value* genSEMFunctionCall(Function& function,
		                                const SEM::Value& semCallValue,
		                                llvm::ArrayRef<SEM::Value> args,
		                                llvm::Value* const hintResultValue) {
			auto& module = function.module();
			
			IREmitter irEmitter(function);
			ValueEmitter valueEmitter(irEmitter);
			
			const auto functionType = semCallValue.type()->asFunctionType();
			
			llvm::SmallVector<llvm_abi::TypedValue, 10> parameters;
			parameters.reserve(args.size());
			
			// Make sure to evaluate arguments first.
			for (const auto& param: args) {
				const auto paramType = param.type();
				const auto abiType = genABIArgType(module, paramType);
				const auto irValue = valueEmitter.emitValue(param);
				parameters.push_back(llvm_abi::TypedValue(irValue,
				                                          abiType));
			}
			
			const auto callInfo = genFunctionCallInfo(function, semCallValue);
			return genFunctionCall(function, functionType, callInfo,
			                       parameters, hintResultValue);
		}
		
		llvm::Value* genRawFunctionCall(Function& function,
		                                const ArgInfo& argInfo,
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
			return genRawFunctionCall(function,
			                          argInfo,
			                          functionPtr,
			                          abiArgs,
			                          musttail);
		}
		
		llvm::Value* genRawFunctionCall(Function& function,
		                                const ArgInfo& argInfo,
		                                llvm::Value* functionPtr,
		                                llvm::ArrayRef<llvm_abi::TypedValue> args,
		                                const bool musttail) {
			auto& module = function.module();
			
			if (argInfo.isVarArg()) {
				assert(args.size() >= argInfo.numArguments());
			} else {
				assert(args.size() == argInfo.numArguments());
			}
			
			IREmitter irEmitter(function);
			
			const auto functionABIType = argInfo.getABIFunctionType();
			const auto functionIRType = argInfo.makeFunctionType();
			
			const auto callBuilder = [&](llvm::ArrayRef<llvm::Value*> encodedArgs) -> llvm::Value* {
				// If the function can throw AND we have pending unwind actions,
				// emit a landing pad to execute those actions.
				if (!argInfo.noExcept() && anyUnwindActions(function, UnwindStateThrow)) {
					assert(!musttail);
					
					const auto successPath = function.createBasicBlock("");
					const auto failPath = genLandingPad(function, UnwindStateThrow);
					
					const auto invokeInst = irEmitter.emitInvoke(functionIRType,
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
					
					function.selectBasicBlock(successPath);
					return invokeInst;
				} else {
					const auto callInst = irEmitter.emitCall(functionIRType,
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
			
			return module.abi().createCall(function,
			                               functionABIType,
			                               callBuilder,
			                               args);
		}
		
		llvm::Value* genNonVarArgsFunctionCall(Function& function,
		                                       SEM::FunctionType functionType,
		                                       const FunctionCallInfo callInfo,
		                                       PendingResultArray args,
		                                       llvm::Value* const hintResultValue) {
			const auto argInfo = getFunctionArgInfo(function.module(), functionType);
			assert(!argInfo.isVarArg() && "This method doesn't support calling varargs functions.");
			assert(args.size() == argInfo.numStandardArguments());
			
			const auto functionABIType = argInfo.getABIFunctionType();
			llvm::SmallVector<llvm_abi::TypedValue, 10> abiArgs;
			abiArgs.reserve(args.size());
			for (size_t i = 0; i < args.size(); i++) {
				const auto abiTypeIndex = argInfo.standardArgumentOffset() + i;
				abiArgs.push_back(llvm_abi::TypedValue(args[i].resolve(function),
				                                       functionABIType.argumentTypes()[abiTypeIndex]));
			}
			return genFunctionCall(function, functionType, callInfo,
			                       abiArgs, hintResultValue);
		}
		
		llvm::Value* genFunctionCall(Function& function,
		                             SEM::FunctionType functionType,
		                             const FunctionCallInfo callInfo,
		                             llvm::ArrayRef<llvm_abi::TypedValue> args,
		                             llvm::Value* const hintResultValue) {
			auto& module = function.module();
			
			IREmitter irEmitter(function);
			
			const auto argInfo = getFunctionArgInfo(function.module(), functionType);
			
			llvm::SmallVector<llvm_abi::TypedValue, 10> llvmArgs;
			llvmArgs.reserve(3 + args.size());
			
			// Some values (e.g. classes) will be returned
			// by assigning to a pointer passed as the first
			// argument (this deals with the class sizes
			// potentially being unknown).
			llvm::Value* returnVar = nullptr;
			if (argInfo.hasReturnVarArgument()) {
				returnVar = irEmitter.emitAlloca(functionType.returnType(), hintResultValue);
				llvmArgs.push_back(llvm_abi::TypedValue(returnVar,
				                                        llvm_abi::PointerTy));
			}
			
			if (argInfo.isVarArg() && argInfo.hasTemplateGeneratorArgument()) {
				assert(callInfo.templateGenerator != nullptr);
				llvmArgs.push_back(llvm_abi::TypedValue(callInfo.templateGenerator,
				                                        templateGeneratorType(module).first));
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
				                                        templateGeneratorType(module).first));
			}
			
			const auto result = genRawFunctionCall(function,
			                                       argInfo,
			                                       callInfo.functionPtr,
			                                       llvmArgs);
			
			if (argInfo.hasReturnVarArgument()) {
				// As above, if the return value pointer is used,
				// this should be loaded (and used instead).
				return irEmitter.emitMoveLoad(returnVar, functionType.returnType());
			} else {
				return result;
			}
		}
		
		llvm::Value* genTemplateMethodCall(Function& function, const MethodInfo& methodInfo, Optional<PendingResult> methodOwner, PendingResultArray args,
				llvm::Value* const hintResultValue) {
			const auto parentType = methodInfo.parentType;
			assert(parentType->isTemplateVar());
			
			const auto templateVar = parentType->getTemplateVar();
			
			const auto typeInfo = function.getBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) templateVar->index() });
			
			const auto methodHash = CreateMethodNameHash(methodInfo.name);
			const auto methodHashValue = ConstantGenerator(function.module()).getI64(methodHash);
			
			const auto contextPointer = methodOwner ?
			                            methodOwner->resolve(function) :
			                            ConstantGenerator(function.module()).getNull(TypeGenerator(function.module()).getPtrType());
			
			llvm::SmallVector<llvm::Value*, 10> llvmArgs;
			llvmArgs.reserve(args.size());
			for (auto& arg: args) {
				llvmArgs.push_back(arg.resolve(function));
			}
			
			const VirtualObjectComponents objectComponents(getTypeInfoComponents(function, typeInfo), contextPointer);
			const VirtualMethodComponents methodComponents(objectComponents, methodHashValue);
			
			IREmitter irEmitter(function);
			auto& module = function.module();
			return module.virtualCallABI().emitCall(irEmitter,
			                                        methodInfo.functionType,
			                                        methodComponents,
			                                        llvmArgs,
			                                        hintResultValue);
		}
		
		llvm::Value* genMethodCall(Function& function, const MethodInfo& methodInfo, Optional<PendingResult> methodOwner, PendingResultArray args,
				llvm::Value* const hintResultValue) {
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
					
					return genTrivialPrimitiveFunctionCall(function, methodInfo, std::move(newArgs), hintResultValue);
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
					
					return genNonVarArgsFunctionCall(function, methodInfo.functionType,
					                                 callInfo, std::move(args),
					                                 hintResultValue);
				}
			} else {
				return genTemplateMethodCall(function, methodInfo, std::move(methodOwner), std::move(args), hintResultValue);
			}
		}
		
		llvm::Value* genDynamicMethodCall(Function& function, const MethodInfo& methodInfo, PendingResult methodOwner, PendingResultArray args,
				llvm::Value* const hintResultValue) {
			return genMethodCall(function, methodInfo, Optional<PendingResult>(std::move(methodOwner)), std::move(args), hintResultValue);
		}
		
		llvm::Value* genStaticMethodCall(Function& function, const MethodInfo& methodInfo, PendingResultArray args,
				llvm::Value* const hintResultValue) {
			return genMethodCall(function, methodInfo, None, std::move(args), hintResultValue);
		}
		
	}
	
}

