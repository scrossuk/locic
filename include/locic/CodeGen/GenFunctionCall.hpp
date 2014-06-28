#ifndef LOCIC_CODEGEN_FUNCTIONCALL_HPP
#define LOCIC_CODEGEN_FUNCTIONCALL_HPP

#include <vector>

#include <locic/SEM.hpp>

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>

namespace locic {
	
	namespace CodeGen {
		
		struct FunctionCallInfo {
			llvm::Value* functionPtr;
			llvm::Value* templateGenerator;
			llvm::Value* contextPointer;
			
			inline FunctionCallInfo()
				: functionPtr(nullptr),
				templateGenerator(nullptr),
				contextPointer(nullptr) { }
		};
		
		// TODO: reduce number of arguments to this function.
		llvm::Value* genFunctionCall(Function& function, FunctionCallInfo callInfo,
			SEM::Type* functionType, const std::vector<SEM::Value*>& args, boost::optional<llvm::DebugLoc> debugLoc);
		
		llvm::Value* genRawFunctionCall(Function& function, const ArgInfo& argInfo, bool canThrow, llvm::Value* functionPtr,
			llvm::ArrayRef<llvm::Value*> args, boost::optional<llvm::DebugLoc> debugLoc = boost::none);
		
	}
	
}

#endif
