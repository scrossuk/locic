#include <cassert>

#include <locic/AST/FunctionType.hpp>
#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/CallEmitter.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/FunctionTranslationStub.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/Support/Utils.hpp>

namespace locic {

	namespace CodeGen {
		
		bool areTypesFunctionallyEquivalent(Module& module,
		                                    const AST::Type* const firstType,
		                                    const AST::Type* const secondType) {
			TypeInfo typeInfo(module);
			return typeInfo.isPassedByValue(firstType) == typeInfo.isPassedByValue(secondType);
		}
		
		bool doFunctionTypesMatch(Module& module,
		                          AST::FunctionType firstType,
		                          AST::FunctionType secondType) {
			assert(firstType.attributes().isTemplated() == secondType.attributes().isTemplated());
			
			const auto& firstParameterTypes = firstType.parameterTypes();
			const auto& secondParameterTypes = secondType.parameterTypes();
			assert(firstParameterTypes.size() == secondParameterTypes.size());
			
			for (size_t i = 0; i < firstParameterTypes.size(); i++) {
				const auto& firstParameterType = firstParameterTypes[i];
				const auto& secondParameterType = secondParameterTypes[i];
				if (!areTypesFunctionallyEquivalent(module, firstParameterType, secondParameterType)) {
					return false;
				}
			}
			
			return areTypesFunctionallyEquivalent(module, firstType.returnType(), secondType.returnType());
		}
		
		llvm::Value* genTranslatedFunctionPointer(Function& functionGenerator,
		                                          llvm::Function* function,
		                                          AST::FunctionType functionType,
		                                          AST::FunctionType translatedFunctionType) {
			assert(functionType.attributes().isTemplated() == translatedFunctionType.attributes().isTemplated());
			
			auto& module = functionGenerator.module();
			
			if (doFunctionTypesMatch(module, functionType, translatedFunctionType)) {
				return function;
			} else {
				return genFunctionTranslationStub(module, function, functionType, translatedFunctionType);
			}
		}
		
		llvm::Value*
		emitTranslate(IREmitter& irEmitter, llvm::Value* const value,
		              const AST::Type* const fromType, const AST::Type* const toType,
		              llvm::Value* const resultPtr) {
			auto& module = irEmitter.module();
			TypeInfo typeInfo(module);
			
			if (typeInfo.isPassedByValue(fromType) && !typeInfo.isPassedByValue(toType)) {
				// Create an alloca to hold the parameter so it can be passed by pointer
				// into the target function.
				const auto argAlloca = irEmitter.emitAlloca(fromType, resultPtr);
				irEmitter.emitStore(value, argAlloca, fromType);
				return argAlloca;
			} else if (!typeInfo.isPassedByValue(fromType) && typeInfo.isPassedByValue(toType)) {
				return irEmitter.emitLoad(value, toType);
			} else {
				return value;
			}
		}
		
		using TranslatedArguments = llvm::SmallVector<llvm_abi::TypedValue, 10>;
		
		TranslatedArguments
		emitTranslateArguments(IREmitter& irEmitter, const Function& functionGenerator,
		                       const AST::FunctionType functionType,
		                       const AST::FunctionType translatedFunctionType) {
			TranslatedArguments arguments;
			
			const auto& parameterTypes = functionType.parameterTypes();
			const auto& translatedParameterTypes = translatedFunctionType.parameterTypes();
			assert(parameterTypes.size() == translatedParameterTypes.size());
			
			for (size_t i = 0; i < parameterTypes.size(); i++) {
				const auto argValue = functionGenerator.getArg(i);
				const auto& fromType = translatedParameterTypes[i];
				const auto& toType = parameterTypes[i];
				
				const auto translatedArg = emitTranslate(irEmitter, argValue,
				                                         fromType, toType,
				                                         /*resultPtr=*/nullptr);
				const auto argType = genABIArgType(irEmitter.module(), toType);
				arguments.push_back(llvm_abi::TypedValue(translatedArg, argType));
			}
			
			return arguments;
		}
		
		llvm::Value*
		emitTranslateResult(IREmitter& irEmitter, llvm::Value* const result,
		                    const AST::FunctionType functionType,
		                    const AST::FunctionType translatedFunctionType,
		                    llvm::Value* const resultPtr) {
			const auto& fromType = functionType.returnType();
			const auto& toType = translatedFunctionType.returnType();
			
			return emitTranslate(irEmitter, result, fromType, toType,
			                     resultPtr);
		}
		
		llvm::Function* createTranslationStubFunction(llvm::Function* function,
		                                              const ArgInfo& translatedArgInfo) {
			const auto llvmFunction =
				translatedArgInfo.createFunction(std::string("translateStub_") + function->getName(),
				                                 llvm::Function::InternalLinkage);
			
			// Always inline if possible.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			return llvmFunction;
		}
		
		llvm::Function* genFunctionTranslationStub(Module& module,
		                                           llvm::Function* function,
		                                           AST::FunctionType functionType,
		                                           AST::FunctionType translatedFunctionType) {
			
			const auto translatedArgInfo = ArgInfo::FromAST(module, translatedFunctionType);
			const auto llvmTranslatedFunctionType = translatedArgInfo.makeFunctionType();
			
			const auto stubIdPair = std::make_pair(function, llvmTranslatedFunctionType);
			const auto iterator = module.functionPtrStubMap().find(stubIdPair);
			if (iterator != module.functionPtrStubMap().end()) {
				return iterator->second;
			}
			
			const auto argInfo = ArgInfo::FromAST(module, functionType);
			
			const auto llvmFunction = createTranslationStubFunction(function, translatedArgInfo);
			
			module.functionPtrStubMap().insert(std::make_pair(stubIdPair, llvmFunction));
			
			Function functionGenerator(module, *llvmFunction, translatedArgInfo);
			IREmitter irEmitter(functionGenerator);
			
			FunctionCallInfo callInfo;
			callInfo.functionPtr = function;
			if (translatedArgInfo.hasTemplateGeneratorArgument()) {
				callInfo.templateGenerator = functionGenerator.getTemplateGenerator();
			}
			if (translatedArgInfo.hasContextArgument()) {
				callInfo.contextPointer = functionGenerator.getContextValue();
			}
			
			const auto arguments = emitTranslateArguments(irEmitter, functionGenerator,
			                                              functionType, translatedFunctionType);
			
			CallEmitter callEmitter(irEmitter);
			const auto result = callEmitter.emitCall(functionType, callInfo, arguments,
			                                         functionGenerator.getReturnVarOrNull());
			
			const auto translatedResult = emitTranslateResult(irEmitter, result,
			                                                  functionType, translatedFunctionType,
			                                                  functionGenerator.getReturnVarOrNull());
			
			irEmitter.emitReturn(translatedResult);
			
			return llvmFunction;
		}
		
	}
	
}

