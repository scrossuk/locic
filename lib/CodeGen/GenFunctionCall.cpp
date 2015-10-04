#include <vector>

#include <llvm-abi/ABI.hpp>
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
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/MethodInfo.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VirtualCall.hpp>
#include <locic/CodeGen/VTable.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {

	namespace CodeGen {
		
		static llvm::Value* decodeReturnValue(Function& function, llvm::Value* const value, llvm_abi::Type* const type, llvm::Type* const llvmType) {
			std::vector<llvm_abi::Type*> abiTypes;
			abiTypes.push_back(type);
			
			std::vector<llvm::Value*> values;
			values.push_back(value);
			function.decodeABIValues(values, abiTypes, {llvmType});
			return values.at(0);
		}
		
		// TODO: merge the duplicated code in this function into genFunctionCall().
		llvm::Value* genSEMFunctionCall(Function& function,
		                                const SEM::Value& semCallValue,
		                                llvm::ArrayRef<SEM::Value> args,
		                                llvm::Value* const hintResultValue) {
			auto& module = function.module();
			auto& abiContext = module.abiContext();
			
			IREmitter irEmitter(function, hintResultValue);
			
			const auto functionType = semCallValue.type()->asFunctionType();
			const auto returnType = functionType.returnType();
			
			const auto functionIRType = getFunctionArgInfo(module, functionType).makeFunctionType();
			
			llvm::SmallVector<llvm::Value*, 16> evaluatedArguments;
			evaluatedArguments.reserve(args.size());
			
			// Make sure to evaluate arguments first.
			for (const auto& param: args) {
				evaluatedArguments.push_back(genValue(function, param));
			}
			
			const auto callInfo = genFunctionCallInfo(function, semCallValue);
			
			std::vector<llvm::Value*> parameters;
			parameters.reserve(3 + args.size());
			
			llvm::SmallVector<llvm_abi::Type*, 10> parameterABITypes;
			
			// Some values (e.g. classes) will be returned
			// by assigning to a pointer passed as the first
			// argument (this deals with the class sizes
			// potentially being unknown).
			llvm::Value* returnVarValue = nullptr;
			
			if (!canPassByValue(module, returnType)) {
				returnVarValue = hintResultValue != nullptr ? hintResultValue : genAlloca(function, returnType);
				parameters.push_back(returnVarValue);
				parameterABITypes.push_back(llvm_abi::Type::Pointer(abiContext));
			}
			
			if (callInfo.templateGenerator != nullptr) {
				parameters.push_back(callInfo.templateGenerator);
				parameterABITypes.push_back(templateGeneratorType(module).first);
			}
			
			if (callInfo.contextPointer != nullptr) {
				parameters.push_back(callInfo.contextPointer);
				parameterABITypes.push_back(llvm_abi::Type::Pointer(abiContext));
			}
			
			const auto intType = getBasicPrimitiveType(module, PrimitiveInt);
			const auto intSize = module.abi().typeSize(getBasicPrimitiveABIType(module, PrimitiveInt));
			const auto doubleSize = module.abi().typeSize(getBasicPrimitiveABIType(module, PrimitiveDouble));
			
			for (size_t i = 0; i < args.size(); i++) {
				const auto paramType = args[i].type();
				auto argValue = evaluatedArguments[i];
				llvm_abi::Type* argABIType = genABIArgType(module, paramType);
				
				// When calling var-args functions, all 'char' and 'short'
				// values must be extended to 'int' values, and all 'float'
				// values must be converted to 'double' values.
				if (functionType.isVarArg() && paramType->isPrimitive()) {
					const auto typeSize = module.abi().typeSize(argABIType);
					
					if (argABIType->isInteger() && typeSize < intSize) {
						if (isSignedIntegerType(module, paramType)) {
							// Need to extend to int (i.e. sign extend).
							argValue = function.getBuilder().CreateSExt(argValue, intType);
						} else if (isUnsignedIntegerType(module, paramType)) {
							// Need to extend to unsigned int (i.e. zero extend).
							argValue = function.getBuilder().CreateZExt(argValue, intType);
						}
						argABIType = llvm_abi::Type::Integer(abiContext, llvm_abi::Int);
					} else if (argABIType->isFloatingPoint() && typeSize < doubleSize) {
						// Need to extend to double.
						argValue = function.getBuilder().CreateFPExt(argValue, TypeGenerator(module).getDoubleType());
						argABIType = llvm_abi::Type::FloatingPoint(abiContext, llvm_abi::Double);
					}
				}
				
				parameters.push_back(argValue);
				parameterABITypes.push_back(argABIType);
			}
			
			function.encodeABIValues(parameters, parameterABITypes);
			
			llvm::Value* encodedCallReturnValue = nullptr;
			
			// Some functions will only be noexcept in certain cases but for
			// CodeGen purposes we're looking for a guarantee of noexcept
			// in all cases, hence we look for always-true noexcept predicates.
			if (!functionType.attributes().noExceptPredicate().isTrivialBool()) {
				assert(!functionType.attributes().noExceptPredicate().dependsOnOnly({}));
			}
			
			// If the function can throw and we have pending unwind actions, generate a
			// landing pad to execute those actions.
			if (!functionType.attributes().noExceptPredicate().isTrue() && anyUnwindActions(function, UnwindStateThrow)) {
				const auto successPath = function.createBasicBlock("");
				const auto failPath = genLandingPad(function, UnwindStateThrow);
				
				encodedCallReturnValue = irEmitter.emitInvoke(functionIRType,
				                                              callInfo.functionPtr,
				                                              successPath,
				                                              failPath,
				                                              parameters);
				
				function.selectBasicBlock(successPath);
			} else {
				encodedCallReturnValue = irEmitter.emitCall(functionIRType,
				                                            callInfo.functionPtr,
				                                            parameters);
			}
			
			if (returnVarValue != nullptr) {
				// As above, if the return value pointer is used,
				// this should be loaded (and used instead).
				return genMoveLoad(function, returnVarValue, returnType);
			} else {
				return decodeReturnValue(function, encodedCallReturnValue, genABIType(function.module(), returnType), genType(function.module(), returnType));
			}
		}
		
		llvm::Value* genRawFunctionCall(Function& function,
		                                const ArgInfo& argInfo,
		                                llvm::Value* functionPtr,
		                                llvm::ArrayRef<llvm::Value*> args) {
			
			assert(args.size() == argInfo.argumentTypes().size());
			
			IREmitter irEmitter(function);
			
			const auto functionType = argInfo.makeFunctionType();
			
			llvm::SmallVector<llvm_abi::Type*, 10> argABITypes;
			argABITypes.reserve(argInfo.argumentTypes().size());
			
			for (const auto& typePair: argInfo.argumentTypes()) {
				argABITypes.push_back(typePair.first);
			}
			
			// Parameters need to be encoded according to the ABI.
			std::vector<llvm::Value*> encodedParameters = args.vec();
			function.encodeABIValues(encodedParameters, argABITypes);
			
			llvm::Value* encodedCallReturnValue = nullptr;
			
			if (!argInfo.noExcept() && anyUnwindActions(function, UnwindStateThrow)) {
				const auto successPath = function.createBasicBlock("");
				const auto failPath = genLandingPad(function, UnwindStateThrow);
				
				const auto invokeInst = irEmitter.emitInvoke(functionType,
				                                             functionPtr,
				                                             successPath,
				                                             failPath,
				                                             encodedParameters);
				
				if (argInfo.noReturn()) {
					invokeInst->setDoesNotReturn();
				}
				
				if (argInfo.noMemoryAccess()) {
					invokeInst->setDoesNotAccessMemory();
				}
				
				encodedCallReturnValue = invokeInst;
				
				function.selectBasicBlock(successPath);
			} else {
				const auto callInst = irEmitter.emitCall(functionType,
				                                         functionPtr,
				                                         encodedParameters);
				
				if (argInfo.noExcept()) {
					callInst->setDoesNotThrow();
				}
				
				if (argInfo.noReturn()) {
					callInst->setDoesNotReturn();
				}
				
				if (argInfo.noMemoryAccess()) {
					callInst->setDoesNotAccessMemory();
				}
				
				encodedCallReturnValue = callInst;
			}
			
			// Return values need to be decoded according to the ABI.
			return decodeReturnValue(function, encodedCallReturnValue, argInfo.returnType().first, argInfo.returnType().second);
		}
		
		llvm::Value* genFunctionCall(Function& function, SEM::FunctionType functionType, const FunctionCallInfo callInfo,
		                             PendingResultArray args, llvm::Value* const hintResultValue) {
			const auto argInfo = getFunctionArgInfo(function.module(), functionType);
			
			llvm::SmallVector<llvm::Value*, 10> llvmArgs;
			
			llvm::Value* returnVar = nullptr;
			if (argInfo.hasReturnVarArgument()) {
				returnVar = genAlloca(function, functionType.returnType(), hintResultValue);
				llvmArgs.push_back(returnVar);
			}
			
			if (argInfo.hasTemplateGeneratorArgument()) {
				assert(callInfo.templateGenerator != nullptr);
				llvmArgs.push_back(callInfo.templateGenerator);
			}
			
			if (argInfo.hasContextArgument()) {
				assert(callInfo.contextPointer != nullptr);
				llvmArgs.push_back(callInfo.contextPointer);
			}
			
			for (auto& pendingResult: args) {
				llvmArgs.push_back(pendingResult.resolve(function));
			}
			
			const auto result = genRawFunctionCall(function, argInfo, callInfo.functionPtr, llvmArgs);
			
			if (argInfo.hasReturnVarArgument()) {
				return genMoveLoad(function, returnVar, functionType.returnType());
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
			
			const auto contextPointer = methodOwner ? methodOwner->resolve(function) : ConstantGenerator(function.module()).getNull(TypeGenerator(function.module()).getPtrType());
			
			llvm::SmallVector<llvm::Value*, 10> llvmArgs;
			llvmArgs.reserve(args.size());
			for (auto& arg: args) {
				llvmArgs.push_back(arg.resolve(function));
			}
			
			const VirtualObjectComponents objectComponents(getTypeInfoComponents(function, typeInfo), contextPointer);
			const VirtualMethodComponents methodComponents(objectComponents, methodHashValue);
			return VirtualCall::generateCall(function, methodInfo.functionType, methodComponents, llvmArgs, hintResultValue);
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
					
					const auto targetFunction = type->getObjectType()->functions().at(CanonicalizeMethodName(methodInfo.name)).get();
					callInfo.functionPtr = genFunctionRef(function, type, targetFunction, methodInfo.functionType);
					
					if (!type->getObjectType()->templateVariables().empty()) {
						callInfo.templateGenerator = getTemplateGenerator(function, TemplateInst::Type(type));
					}
					
					if (methodOwner) {
						const auto i8PtrType = TypeGenerator(function.module()).getPtrType();
						const auto resolvedMethodOwner = methodOwner->resolve(function);
						callInfo.contextPointer = function.getBuilder().CreatePointerCast(resolvedMethodOwner, i8PtrType);
					}
					
					return genFunctionCall(function, methodInfo.functionType, callInfo, std::move(args), hintResultValue);
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

