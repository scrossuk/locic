#include <cassert>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionTranslationStub.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
		
		static bool checkImplies(const bool a, const bool b) {
			return !a || b;
		}
		
		using TranslatedArguments = llvm::SmallVector<llvm::Value*, 10>;
		
		TranslatedArguments getTranslatedArguments(Function& functionGenerator,
		                                           const SEM::Type* const functionType,
		                                           const SEM::Type* const translatedFunctionType,
		                                           llvm::Value* const returnVar,
		                                           const ArgInfo& argInfo,
		                                           const ArgInfo& translatedArgInfo) {
			auto& module = functionGenerator.module();
			auto& builder = functionGenerator.getBuilder();
			
			TranslatedArguments args;
			
			if (argInfo.hasReturnVarArgument()) {
				const auto llvmReturnType = genArgType(module, functionType->getFunctionReturnType());
				args.push_back(builder.CreatePointerCast(returnVar, llvmReturnType));
			}
			
			if (translatedArgInfo.hasTemplateGeneratorArgument()) {
				args.push_back(functionGenerator.getTemplateGenerator());
			}
			
			if (translatedArgInfo.hasContextArgument()) {
				args.push_back(functionGenerator.getRawContextValue());
			}
			
			const auto& parameterTypes = functionType->getFunctionParameterTypes();
			const auto& translatedParameterTypes = translatedFunctionType->getFunctionParameterTypes();
			assert(parameterTypes.size() == translatedParameterTypes.size());
			
			for (size_t i = 0; i < parameterTypes.size(); i++) {
				const auto parameterType = parameterTypes[i];
				const auto translatedParameterType = translatedParameterTypes[i];
				
				const auto llvmParameterType = genArgType(module, parameterType);
				const auto llvmTranslatedParameterType = genArgType(module, translatedParameterType);
				
				// Being able to pass the inner parameter type by value must imply
				// that the outer parameter type can be passed by value.
				assert(checkImplies(canPassByValue(module, parameterType), canPassByValue(module, translatedParameterType)));
				
				const auto argValue = functionGenerator.getArg(i);
				
				if (!canPassByValue(module, parameterType) && canPassByValue(module, translatedParameterType)) {
					// Create an alloca to hinner the parameter so it can be passed by pointer
					// into the target function.
					const auto argAlloca = genAlloca(functionGenerator, translatedParameterType);
					genStore(functionGenerator, argValue, argAlloca, translatedParameterType);
					args.push_back(builder.CreatePointerCast(argAlloca, llvmParameterType));
				} else if (llvmParameterType->isPointerTy() && llvmTranslatedParameterType->isPointerTy()) {
					// Make sure our pointers have the right type.
					args.push_back(builder.CreatePointerCast(argValue, llvmParameterType));
				} else {
					args.push_back(argValue);
				}
			}
			
			return args;
		}
		
		llvm::Function* createTranslationStubFunction(Module& module,
		                                              llvm::Function* function,
		                                              const ArgInfo& translatedArgInfo) {
			const auto functionName = module.getCString("translateStub_") + function->getName();
			const auto linkage = llvm::Function::InternalLinkage;
			return createLLVMFunction(module, translatedArgInfo, linkage, functionName);
		}
		
		llvm::Function* genFunctionTranslationStub(Module& module,
		                                           llvm::Function* function,
		                                           const SEM::Type* const functionType,
		                                           const SEM::Type* const translatedFunctionType) {
			assert(functionType->isFunction());
			assert(translatedFunctionType->isFunction());
			
			const auto llvmTranslatedFunctionType = genFunctionType(module, translatedFunctionType);
			
			const auto stubIdPair = std::make_pair(function, llvmTranslatedFunctionType);
			const auto iterator = module.functionPtrStubMap().find(stubIdPair);
			if (iterator != module.functionPtrStubMap().end()) {
				return iterator->second;
			}
			
			const auto argInfo = getFunctionArgInfo(module, functionType);
			const auto translatedArgInfo = getFunctionArgInfo(module, translatedFunctionType);
			
			const auto llvmFunction = createTranslationStubFunction(module, function, translatedArgInfo);
			
			module.functionPtrStubMap().insert(std::make_pair(stubIdPair, llvmFunction));
			
			// Always inline if possible.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			Function functionGenerator(module, *llvmFunction, translatedArgInfo);
			auto& builder = functionGenerator.getBuilder();
			
			const auto returnVar =
				argInfo.hasReturnVarArgument() ?
					translatedArgInfo.hasReturnVarArgument() ?
						functionGenerator.getReturnVar() :
						genAlloca(functionGenerator, translatedFunctionType->getFunctionReturnType())
					: nullptr;
			
			TranslatedArguments arguments = getTranslatedArguments(functionGenerator,
			                                                       functionType,
			                                                       translatedFunctionType,
			                                                       returnVar,
			                                                       argInfo,
			                                                       translatedArgInfo);
			
			const auto result = genRawFunctionCall(functionGenerator, argInfo, function, arguments);
			
			if (argInfo.hasReturnVarArgument() && !translatedArgInfo.hasReturnVarArgument()) {
				builder.CreateRet(builder.CreateLoad(returnVar));
			} else {
				if (llvmTranslatedFunctionType->getReturnType()->isVoidTy()) {
					builder.CreateRetVoid();
				} else {
					if (result->getType()->isPointerTy()) {
						builder.CreateRet(builder.CreatePointerCast(result, llvmTranslatedFunctionType->getReturnType()));
					} else {
						builder.CreateRet(result);
					}
				}
			}
			
			return llvmFunction;
		}
		
		bool areTypesFunctionallyEquivalent(Module& module,
		                                    const SEM::Type* const firstType,
		                                    const SEM::Type* const secondType) {
			return firstType == secondType ||
				genArgType(module, firstType) == genArgType(module, secondType);
		}
		
		bool doFunctionTypesMatch(Module& module,
		                          const SEM::Type* const firstType,
		                          const SEM::Type* const secondType) {
			assert(firstType->isFunction());
			assert(secondType->isFunction());
			assert(firstType->isFunctionTemplated() == secondType->isFunctionTemplated());
			
			const auto& firstParameterTypes = firstType->getFunctionParameterTypes();
			const auto& secondParameterTypes = secondType->getFunctionParameterTypes();
			assert(firstParameterTypes.size() == secondParameterTypes.size());
			
			for (size_t i = 0; i < firstParameterTypes.size(); i++) {
				const auto& firstParameterType = firstParameterTypes[i];
				const auto& secondParameterType = secondParameterTypes[i];
				if (!areTypesFunctionallyEquivalent(module, firstParameterType, secondParameterType)) {
					return false;
				}
			}
			
			return areTypesFunctionallyEquivalent(module, firstType->getFunctionReturnType(), secondType->getFunctionReturnType());
		}
		
		llvm::Value* genTranslatedFunctionPointer(Function& functionGenerator,
		                                          llvm::Function* function,
		                                          const SEM::Type* const functionType,
		                                          const SEM::Type* const translatedFunctionType) {
			assert(functionType->isFunction());
			assert(translatedFunctionType->isFunction());
			assert(functionType->isFunctionTemplated() == translatedFunctionType->isFunctionTemplated());
			
			auto& module = functionGenerator.module();
			
			if (doFunctionTypesMatch(module, functionType, translatedFunctionType)) {
				const auto functionPtrType = genFunctionType(module, translatedFunctionType)->getPointerTo();
				return functionGenerator.getBuilder().CreatePointerCast(function, functionPtrType);
			} else {
				// This case means that the function is templated on the
				// return type and a primitive has been passed for that
				// type argument, so the original function will have accepted
				// a return var but the function reference expects it
				// to return the primitive directly. We need to fix this
				// by creating a stub that translates between them.
				return genFunctionTranslationStub(module, function, functionType, translatedFunctionType);
			}
		}
		
	}
	
}

