#ifndef LOCIC_CODEGEN_FUNCTIONCALL_HPP
#define LOCIC_CODEGEN_FUNCTIONCALL_HPP

#include <locic/CodeGen/PendingResult.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace SEM {
		
		class Type;
		class Value;
		
	}
	
	namespace CodeGen {
		
		class Function;
		struct FunctionCallInfo;
		struct MethodInfo;
		
		/**
		 * \brief Generate function call.
		 * 
		 */
		llvm::Value* genFunctionCall(Function& function, FunctionCallInfo callInfo,
			const SEM::Type* functionType, llvm::ArrayRef<SEM::Value> args,
			llvm::Value* hintResultValue = nullptr);
		
		llvm::Value* genRawFunctionCall(Function& function, const ArgInfo& argInfo, llvm::Value* functionPtr,
			llvm::ArrayRef<llvm::Value*> args);
		
		llvm::Value* genMethodCall(Function& function, const MethodInfo& methodInfo, Optional<PendingResult> methodOwner, PendingResultArray args,
			llvm::Value* const hintResultValue = nullptr);
		
		llvm::Value* genDynamicMethodCall(Function& function, const MethodInfo& methodInfo, PendingResult methodOwner, PendingResultArray args,
			llvm::Value* const hintResultValue = nullptr);
		
		llvm::Value* genStaticMethodCall(Function& function, const MethodInfo& methodInfo, PendingResultArray args,
			llvm::Value* const hintResultValue = nullptr);
		
	}
	
}

#endif
