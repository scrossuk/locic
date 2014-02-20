#ifndef LOCIC_CODEGEN_EXCEPTION_HPP
#define LOCIC_CODEGEN_EXCEPTION_HPP

#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Function* getExceptionAllocateFunction(Module& module);
		
		llvm::Function* getExceptionThrowFunction(Module& module);
		
	}
	
}

#endif
