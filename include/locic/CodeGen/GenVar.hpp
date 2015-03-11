#ifndef LOCIC_CODEGEN_GENVAR_HPP
#define LOCIC_CODEGEN_GENVAR_HPP

#include <locic/SEM.hpp>

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>

namespace locic {

	namespace CodeGen {
		
		void genVarAlloca(Function& function, SEM::Var* var, Optional<llvm::DebugLoc> debugLoc = None);
		
		void genVarInitialise(Function& function, SEM::Var* var, llvm::Value* initialiseValue, Optional<llvm::DebugLoc> debugLoc = None);
		
	}
	
}

#endif
