#include <locic/CodeGen/ASTCodeEmitter.hpp>

#include <locic/AST/Function.hpp>
#include <locic/AST/Type.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/DefaultMethodEmitter.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenVar.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/PrimitiveFunctionEmitter.hpp>
#include <locic/CodeGen/ScopeEmitter.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/Support.hpp>

#include <locic/AST/TypeInstance.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		ASTCodeEmitter::ASTCodeEmitter(Function& functionGenerator)
		: functionGenerator_(functionGenerator) { }
		
		void
		ASTCodeEmitter::emitFunctionCode(const AST::TypeInstance* const typeInstance,
		                                 const AST::Function& function,
		                                 const bool isInnerMethod) {
			// TODO: remove this horrible code...
			const bool isOuterMethod = (function.fullName().last() == "__moveto" ||
			                            function.fullName().last() == "__destroy") &&
			                           !isInnerMethod;
			if (function.isAutoGenerated() || function.isPrimitive() || isOuterMethod) {
				emitBuiltInFunctionCode(typeInstance,
				                        function,
				                        isInnerMethod);
			} else {
				emitUserFunctionCode(function);
			}
		}
		
		llvm::Value*
		ASTCodeEmitter::emitBuiltInFunctionContents(const MethodID methodID,
		                                            const bool isInnerMethod,
		                                            const AST::TypeInstance* const typeInstance,
		                                            const AST::Function& function,
		                                            PendingResultArray args,
		                                            llvm::Value* const hintResultValue) {
			if (!function.isPrimitive()) {
				DefaultMethodEmitter defaultMethodEmitter(functionGenerator_);
				return defaultMethodEmitter.emitMethod(methodID,
				                                       isInnerMethod,
				                                       typeInstance->selfType(),
				                                       function.type(),
				                                       std::move(args),
				                                       hintResultValue);
			} else {
				assert(!isInnerMethod);
				
				AST::ValueArray templateArgs;
				for (const auto& templateVar: function.templateVariables()) {
					templateArgs.push_back(templateVar->selfRefValue());
				}
				
				const auto type = typeInstance != nullptr ? typeInstance->selfType() : nullptr;
				
				IREmitter irEmitter(functionGenerator_);
				PrimitiveFunctionEmitter primitiveFunctionEmitter(irEmitter);
				return primitiveFunctionEmitter.emitFunction(methodID, type,
				                                             arrayRef(templateArgs),
				                                             std::move(args),
				                                             hintResultValue);
			}
		}
		
		void
		ASTCodeEmitter::emitBuiltInFunctionCode(const AST::TypeInstance* const typeInstance,
		                                        const AST::Function& function,
		                                        const bool isInnerMethod) {
			auto& module = functionGenerator_.module();
			
			const auto& argInfo = functionGenerator_.getArgInfo();
			
			PendingResultArray args;
			
			Array<RefPendingResult, 1> contextPendingResult;
			if (argInfo.hasContextArgument()) {
				assert(typeInstance != nullptr);
				const auto contextValue = functionGenerator_.getContextValue();
				contextPendingResult.push_back(RefPendingResult(contextValue,
				                                                typeInstance->selfType()));
				args.push_back(contextPendingResult.back());
			}
			
			// Need an array to store all the pending results
			// being referred to in 'genTrivialPrimitiveFunctionCall'.
			Array<ValuePendingResult, 10> pendingResultArgs;
			
			const auto& argTypes = function.type().parameterTypes();
			for (size_t i = 0; i < argTypes.size(); i++) {
				const auto argValue = functionGenerator_.getArg(i);
				pendingResultArgs.push_back(ValuePendingResult(argValue, argTypes[i]));
				args.push_back(pendingResultArgs.back());
			}
			
			const auto& methodName = function.fullName().last();
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			const auto hintResultValue = functionGenerator_.getReturnVarOrNull();
			
			const auto result = emitBuiltInFunctionContents(methodID,
			                                                isInnerMethod,
			                                                typeInstance,
			                                                function,
			                                                std::move(args),
			                                                hintResultValue);
			
			const auto returnType = function.type().returnType();
			
			IREmitter irEmitter(functionGenerator_);
			
			// Return the result in the appropriate way.
			if (argInfo.hasReturnVarArgument()) {
				irEmitter.emitMoveStore(result,
				                        functionGenerator_.getReturnVar(),
				                        returnType);
				irEmitter.emitReturnVoid();
			} else if (!returnType->isBuiltInVoid()) {
				functionGenerator_.returnValue(result);
			} else {
				irEmitter.emitReturnVoid();
			}
		}
		
		void
		ASTCodeEmitter::emitUserFunctionCode(const AST::Function& function) {
			assert(!function.isAutoGenerated());
			
			emitParameterAllocas(function);
			emitScopeFunctionCode(function);
		}
		
		void
		ASTCodeEmitter::emitParameterAllocas(const AST::Function& function) {
			auto& module = functionGenerator_.module();
			
			for (size_t i = 0; i < function.parameters().size(); i++) {
				const auto& paramVar = function.parameters()[i];
				
				// If value can't be passed by value it will be
				// passed by pointer, which can be re-used for
				// a variable to avoid an alloca.
				const auto hintResultValue =
					canPassByValue(module, paramVar->constructType()) ?
						nullptr :
						functionGenerator_.getArg(i);
				
				genVarAlloca(functionGenerator_, paramVar, hintResultValue);
			}
		}
		
		void
		ASTCodeEmitter::emitScopeFunctionCode(const AST::Function& function) {
			ScopeLifetime functionScopeLifetime(functionGenerator_);
			
			// Parameters need to be copied to the stack, so that it's
			// possible to assign to them, take their address, etc.
			const auto& parameterVars = function.parameters();
			
			for (size_t i = 0; i < parameterVars.size(); i++) {
				const auto paramVar = parameterVars.at(i);
				
				// Get the variable's alloca.
				const auto stackObject = functionGenerator_.getLocalVarMap().get(paramVar);
				
				// Store the argument into the stack alloca.
				genStoreVar(functionGenerator_,
				            functionGenerator_.getArg(i),
				            stackObject,
				            paramVar);
				
				// Add this to the list of variables to be
				// destroyed at the end of the function.
				scheduleDestructorCall(functionGenerator_,
				                       paramVar->lvalType(),
				                       stackObject);
			}
			
			IREmitter irEmitter(functionGenerator_);
			ScopeEmitter(irEmitter).emitScope(*(function.scope()));
		}
		
	}
	
}