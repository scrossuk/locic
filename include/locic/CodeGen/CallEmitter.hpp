#ifndef LOCIC_CODEGEN_CALLEMITTER_HPP
#define LOCIC_CODEGEN_CALLEMITTER_HPP

#include <llvm-abi/TypedValue.hpp>

#include <locic/CodeGen/PendingResult.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace AST {
		
		class FunctionType;
		class Type;
		class Value;
		
	}
	
	namespace CodeGen {
		
		class ArgInfo;
		struct FunctionCallInfo;
		class IREmitter;
		struct MethodInfo;
		
		class CallEmitter {
		public:
			CallEmitter(IREmitter& irEmitter);
			
			/**
			 * \brief Emit AST function call.
			 * 
			 * Calls the given function with the AST::Value arguments.
			 */
			llvm::Value*
			emitASTCall(const AST::Value& astCallValue,
			            llvm::ArrayRef<AST::Value> args,
			            llvm::Value* resultPtr = nullptr);
			
			/**
			 * \brief Emit raw function call.
			 * 
			 * This generates a call with arguments pre-prepared, i.e. this
			 * function doesn't handle passing template generators, context
			 * pointers or return value pointers; these must be handled at
			 * a higher level.
			 */
			llvm::Value*
			emitRawCall(const ArgInfo& argInfo, llvm::Value* functionPtr,
			            llvm::ArrayRef<llvm::Value*> args,
			            bool musttail = false);
			
			llvm::Value*
			emitRawCall(const ArgInfo& argInfo, llvm::Value* functionPtr,
			            llvm::ArrayRef<llvm_abi::TypedValue> args,
			            bool musttail = false);
			
			llvm::Value*
			emitNonVarArgsCall(AST::FunctionType functionType,
			                   FunctionCallInfo callInfo,
			                   PendingResultArray args,
			                   llvm::Value* resultPtr);
			
			/**
			 * \brief Emit function call.
			 * 
			 * This generates a call with the given information, which
			 * specified the context pointer (i.e. the 'this' pointer),
			 * the template generator and the function pointer itself.
			 * 
			 * Using this information this function generates a call which
			 * correctly passes the special aspects of a call (such as the
			 * template generator).
			 */
			llvm::Value*
			emitCall(AST::FunctionType functionType,
			                      const FunctionCallInfo callInfo,
			                      llvm::ArrayRef<llvm_abi::TypedValue> args,
			                      llvm::Value* resultPtr);
			
			llvm::Value*
			emitTemplateMethodCall(const MethodInfo& methodInfo,
			                       Optional<PendingResult> methodOwner,
			                       PendingResultArray args,
			                       llvm::Value* resultPtr);
			
			llvm::Value*
			emitMethodCall(const MethodInfo& methodInfo,
			               Optional<PendingResult> methodOwner,
			               PendingResultArray args,
			               llvm::Value* resultPtr = nullptr);
			
			llvm::Value*
			emitDynamicMethodCall(const MethodInfo& methodInfo,
			                      PendingResult methodOwner,
			                      PendingResultArray args,
			                      llvm::Value* resultPtr = nullptr);
			
			llvm::Value*
			emitStaticMethodCall(const MethodInfo& methodInfo,
			                     PendingResultArray args,
			                     llvm::Value* resultPtr = nullptr);
			
		private:
			IREmitter& irEmitter_;
			
		};
		
	}
	
}

#endif
