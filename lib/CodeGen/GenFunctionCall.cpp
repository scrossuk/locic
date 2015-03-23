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
		
		llvm::Value* genFunctionCall(Function& function, FunctionCallInfo callInfo, const SEM::Type* functionType, llvm::ArrayRef<SEM::Value> args,
				llvm::Value* const hintResultValue) {
			assert(callInfo.functionPtr != nullptr);
			
			auto& module = function.module();
			auto& abiContext = module.abiContext();
			
			const auto returnType = functionType->getFunctionReturnType();
			
			const auto llvmFunctionType = callInfo.functionPtr->getType()->getPointerElementType();
			assert(llvmFunctionType->isFunctionTy());
			
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
			
			for (const auto& param: args) {
				llvm::Value* argValue = genValue(function, param);
				llvm_abi::Type* argABIType = genABIArgType(module, param.type());
				
				// When calling var-args functions, all 'char' and 'short'
				// values must be extended to 'int' values, and all 'float'
				// values must be converted to 'double' values.
				if (llvmFunctionType->isFunctionVarArg() && param.type()->isPrimitive()) {
					const auto typeSize = module.abi().typeSize(argABIType);
					
					if (argABIType->isInteger() && typeSize < intSize) {
						if (isSignedIntegerType(module, param.type())) {
							// Need to extend to int (i.e. sign extend).
							argValue = function.getBuilder().CreateSExt(argValue, intType);
						} else if (isUnsignedIntegerType(module, param.type())) {
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
			if (!functionType->functionNoExceptPredicate().isTrivialBool()) {
				assert(!functionType->functionNoExceptPredicate().dependsOnOnly({}));
			}
			
			// If the function can throw and we have pending unwind actions, generate a
			// landing pad to execute those actions.
			if (!functionType->functionNoExceptPredicate().isTrue() && anyUnwindActions(function, UnwindStateThrow)) {
				const auto successPath = function.createBasicBlock("");
				const auto failPath = genLandingPad(function, UnwindStateThrow);
				
				encodedCallReturnValue = function.getBuilder().CreateInvoke(callInfo.functionPtr, successPath, failPath, parameters);
				
				function.selectBasicBlock(successPath);
			} else {
				encodedCallReturnValue = function.getBuilder().CreateCall(callInfo.functionPtr, parameters);
			}
			
			if (returnVarValue != nullptr) {
				// As above, if the return value pointer is used,
				// this should be loaded (and used instead).
				return genMoveLoad(function, returnVarValue, returnType);
			} else {
				return decodeReturnValue(function, encodedCallReturnValue, genABIType(function.module(), returnType), genType(function.module(), returnType));
			}
		}
		
		llvm::Value* genRawFunctionCall(Function& function, const ArgInfo& argInfo, llvm::Value* functionPtr,
				llvm::ArrayRef<llvm::Value*> args) {
			
			assert(args.size() == argInfo.argumentTypes().size());
			
			assert(functionPtr->getType()->getPointerElementType()->isFunctionTy());
			
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
				
				const auto invokeInst = function.getBuilder().CreateInvoke(functionPtr, successPath, failPath, encodedParameters);
				
				if (argInfo.noReturn()) {
					invokeInst->setDoesNotReturn();
				}
				
				if (argInfo.noMemoryAccess()) {
					invokeInst->setDoesNotAccessMemory();
				}
				
				encodedCallReturnValue = invokeInst;
				
				function.selectBasicBlock(successPath);
			} else {
				const auto callInst = function.getBuilder().CreateCall(functionPtr, encodedParameters);
				
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
		
		llvm::Value* genTemplateMethodCall(Function& function, MethodInfo methodInfo, Optional<PendingResult> methodOwner, PendingResultArray args,
				llvm::Value* const hintResultValue) {
			const auto parentType = methodInfo.parentType;
			assert(parentType->isTemplateVar());
			
			const auto templateVar = parentType->getTemplateVar();
			
			const auto typeInfo = function.getBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) templateVar->index() });
			
			const auto methodHash = CreateMethodNameHash(methodInfo.name);
			const auto methodHashValue = ConstantGenerator(function.module()).getI64(methodHash);
			
			const auto contextPointer = methodOwner ? methodOwner->resolve(function) : ConstantGenerator(function.module()).getNull(TypeGenerator(function.module()).getI8PtrType());
			
			llvm::SmallVector<llvm::Value*, 10> llvmArgs;
			llvmArgs.reserve(args.size());
			for (auto& arg: args) {
				llvmArgs.push_back(arg.resolve(function));
			}
			
			const VirtualObjectComponents objectComponents(getTypeInfoComponents(function, typeInfo), contextPointer);
			const VirtualMethodComponents methodComponents(objectComponents, methodHashValue);
			
			if (methodOwner) {
				// A dynamic method.
				assert(methodInfo.functionType->isFunctionMethod());
				const auto interfaceMethodType = SEM::Type::InterfaceMethod(methodInfo.functionType);
				return VirtualCall::generateCall(function, interfaceMethodType, methodComponents, llvmArgs, hintResultValue);
			} else {
				// A static method.
				assert(!methodInfo.functionType->isFunctionMethod());
				const auto staticInterfaceMethodType = SEM::Type::StaticInterfaceMethod(methodInfo.functionType);
				return VirtualCall::generateCall(function, staticInterfaceMethodType, methodComponents, llvmArgs, hintResultValue);
			}
		}
		
		llvm::Value* genMethodCall(Function& function, MethodInfo methodInfo, Optional<PendingResult> methodOwner, PendingResultArray args,
				llvm::Value* const hintResultValue) {
			const auto type = methodInfo.parentType;
			
			assert(type != nullptr);
			assert(type->isObjectOrTemplateVar());
			assert(methodInfo.functionType->isFunction());
			
			if (type->isObject()) {
				if (type->isPrimitive()) {
					PendingResultArray newArgs;
					if (methodOwner) {
						newArgs.push_back(std::move(*methodOwner));
					}
					
					for (auto& arg: args) {
						newArgs.push_back(std::move(arg));
					}
					
					return genTrivialPrimitiveFunctionCall(function, std::move(methodInfo), std::move(newArgs), hintResultValue);
				} else {
					const auto targetFunction = type->getObjectType()->functions().at(CanonicalizeMethodName(methodInfo.name)).get();
					const auto argInfo = getFunctionArgInfo(function.module(), methodInfo.functionType);
					const auto functionPtr = genFunctionRef(function, type, targetFunction, methodInfo.functionType);
					
					llvm::SmallVector<llvm::Value*, 10> llvmArgs;
					
					llvm::Value* returnVar = nullptr;
					if (argInfo.hasReturnVarArgument()) {
						returnVar = hintResultValue != nullptr ? hintResultValue : genAlloca(function, methodInfo.functionType->getFunctionReturnType());
						llvmArgs.push_back(returnVar);
					}
					
					if (methodOwner) {
						llvmArgs.push_back(methodOwner->resolve(function));
					}
					
					for (auto& pendingResult: args) {
						llvmArgs.push_back(pendingResult.resolve(function));
					}
					
					const auto result = genRawFunctionCall(function, argInfo, functionPtr, llvmArgs);
					
					if (argInfo.hasReturnVarArgument()) {
						return genMoveLoad(function, returnVar, methodInfo.functionType->getFunctionReturnType());
					} else {
						return result;
					}
				}
			} else {
				return genTemplateMethodCall(function, std::move(methodInfo), std::move(methodOwner), std::move(args), hintResultValue);
			}
		}
		
		llvm::Value* genDynamicMethodCall(Function& function, MethodInfo methodInfo, PendingResult methodOwner, PendingResultArray args,
				llvm::Value* const hintResultValue) {
			return genMethodCall(function, std::move(methodInfo), Optional<PendingResult>(std::move(methodOwner)), std::move(args), hintResultValue);
		}
		
		llvm::Value* genStaticMethodCall(Function& function, MethodInfo methodInfo, PendingResultArray args,
				llvm::Value* const hintResultValue) {
			return genMethodCall(function, std::move(methodInfo), None, std::move(args), hintResultValue);
		}
		
	}
	
}

