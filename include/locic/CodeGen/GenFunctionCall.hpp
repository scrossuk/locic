#ifndef LOCIC_CODEGEN_FUNCTIONCALL_HPP
#define LOCIC_CODEGEN_FUNCTIONCALL_HPP

#include <vector>

#include <locic/SEM.hpp>

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/MethodInfo.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace CodeGen {
		
		// TODO: reduce number of arguments to this function.
		llvm::Value* genFunctionCall(Function& function, FunctionCallInfo callInfo,
			const SEM::Type* functionType, const std::vector<SEM::Value>& args,
			Optional<llvm::DebugLoc> debugLoc, llvm::Value* hintResultValue = nullptr);
		
		llvm::Value* genRawFunctionCall(Function& function, const ArgInfo& argInfo, llvm::Value* functionPtr,
			llvm::ArrayRef<llvm::Value*> args, Optional<llvm::DebugLoc> debugLoc = None);
		
		llvm::Value* genMethodCall(Function& function, MethodInfo methodInfo, Optional<PendingResult> methodOwner, PendingResultArray args,
			Optional<llvm::DebugLoc> debugLoc = None, llvm::Value* const hintResultValue = nullptr);
		
		llvm::Value* genDynamicMethodCall(Function& function, MethodInfo methodInfo, PendingResult methodOwner, PendingResultArray args,
			Optional<llvm::DebugLoc> debugLoc = None, llvm::Value* const hintResultValue = nullptr);
		
		llvm::Value* genStaticMethodCall(Function& function, MethodInfo methodInfo, PendingResultArray args,
			Optional<llvm::DebugLoc> debugLoc = None, llvm::Value* const hintResultValue = nullptr);
		
	}
	
}

#endif
