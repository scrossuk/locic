#ifndef LOCIC_CODEGEN_FUNCTIONCALL_HPP
#define LOCIC_CODEGEN_FUNCTIONCALL_HPP

#include <vector>

#include <locic/SEM.hpp>

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>

namespace locic {
	
	namespace CodeGen {
		
		// TODO: reduce number of arguments to this function.
		llvm::Value* genFunctionCall(Function& function, llvm::Value* functionValue, llvm::Value* contextPointer,
			SEM::Type* returnType, const std::vector<SEM::Value*>& args, boost::optional<llvm::DebugLoc> debugLoc);
		
	}
	
}

#endif
