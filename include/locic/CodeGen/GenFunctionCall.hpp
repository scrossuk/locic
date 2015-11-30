#ifndef LOCIC_CODEGEN_GENFUNCTIONCALL_HPP
#define LOCIC_CODEGEN_GENFUNCTIONCALL_HPP

#include <llvm-abi/TypedValue.hpp>

#include <locic/CodeGen/PendingResult.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace SEM {
		
		class FunctionType;
		class Type;
		class Value;
		
	}
	
	namespace CodeGen {
		
		class Function;
		struct FunctionCallInfo;
		struct MethodInfo;
		
		/**
		 * \brief Generate SEM function call.
		 * 
		 * Calls the given function with the SEM::Value arguments.
		 */
		llvm::Value* genSEMFunctionCall(Function& function,
		                                const SEM::Value& callValue,
		                                llvm::ArrayRef<SEM::Value> args,
		                                llvm::Value* hintResultValue = nullptr);
		
		/**
		 * \brief Generate raw function call.
		 * 
		 * This generates a call with arguments pre-prepared, i.e. this
		 * function doesn't handle passing template generators, context
		 * pointers or return value pointers; these must be handled at
		 * a higher level.
		 */
		llvm::Value* genRawFunctionCall(Function& function,
		                                const ArgInfo& argInfo,
		                                llvm::Value* functionPtr,
		                                llvm::ArrayRef<llvm_abi::TypedValue> args,
		                                bool musttail = false);
		
		llvm::Value* genRawFunctionCall(Function& function,
		                                const ArgInfo& argInfo,
		                                llvm::Value* functionPtr,
		                                llvm::ArrayRef<llvm::Value*> args,
		                                bool musttail = false);
		
		/**
		 * \brief Generate function call.
		 * 
		 * This generates a call with the given information, which
		 * specified the context pointer (i.e. the 'this' pointer),
		 * the template generator and the function pointer itself.
		 * 
		 * Using this information this function generates a call which
		 * correctly passes the special aspects of a call (such as the
		 * template generator).
		 */
		llvm::Value* genFunctionCall(Function& function, SEM::FunctionType functionType, FunctionCallInfo callInfo,
		                             PendingResultArray args, llvm::Value* const hintResultValue);
		
		llvm::Value* genMethodCall(Function& function, const MethodInfo& methodInfo, Optional<PendingResult> methodOwner, PendingResultArray args,
		                           llvm::Value* const hintResultValue = nullptr);
		
		llvm::Value* genDynamicMethodCall(Function& function, const MethodInfo& methodInfo, PendingResult methodOwner, PendingResultArray args,
		                                  llvm::Value* const hintResultValue = nullptr);
		
		llvm::Value* genStaticMethodCall(Function& function, const MethodInfo& methodInfo, PendingResultArray args,
		                                 llvm::Value* const hintResultValue = nullptr);
		
	}
	
}

#endif
