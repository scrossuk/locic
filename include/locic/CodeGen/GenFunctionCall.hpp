#ifndef LOCIC_CODEGEN_GENFUNCTIONCALL_HPP
#define LOCIC_CODEGEN_GENFUNCTIONCALL_HPP

#include <llvm-abi/TypedValue.hpp>

#include <locic/CodeGen/PendingResult.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace AST {
		
		class FunctionType;
		class Type;
		
	}
	
	namespace CodeGen {
		
		class Function;
		struct FunctionCallInfo;
		struct MethodInfo;
		
		/**
		 * \brief Generate AST function call.
		 * 
		 * Calls the given function with the AST::Value arguments.
		 */
		llvm::Value* genASTFunctionCall(Function& function,
		                                const AST::Value& callValue,
		                                llvm::ArrayRef<AST::Value> args,
		                                llvm::Value* resultPtr = nullptr);
		
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
		llvm::Value* genFunctionCall(Function& function,
		                             AST::FunctionType functionType,
		                             FunctionCallInfo callInfo,
		                             llvm::ArrayRef<llvm_abi::TypedValue> args,
		                             llvm::Value* resultPtr);
		
		llvm::Value* genNonVarArgsFunctionCall(Function& function,
		                                       AST::FunctionType functionType,
		                                       FunctionCallInfo callInfo,
		                                       PendingResultArray args,
		                                       llvm::Value* resultPtr);
		
		llvm::Value* genMethodCall(Function& function, const MethodInfo& methodInfo, Optional<PendingResult> methodOwner, PendingResultArray args,
		                           llvm::Value* const resultPtr = nullptr);
		
		llvm::Value* genDynamicMethodCall(Function& function, const MethodInfo& methodInfo, PendingResult methodOwner, PendingResultArray args,
		                                  llvm::Value* const resultPtr = nullptr);
		
		llvm::Value* genStaticMethodCall(Function& function, const MethodInfo& methodInfo, PendingResultArray args,
		                                 llvm::Value* const resultPtr = nullptr);
		
	}
	
}

#endif
