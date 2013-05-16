#ifndef LOCIC_CODEGEN_SIZEOF_HPP
#define LOCIC_CODEGEN_SIZEOF_HPP

#include <Locic/SEM.hpp>
#include <Locic/CodeGen/Function.hpp>
#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
	
		llvm::Function* genSizeOfFunction(Module& module, SEM::Type* type);
		
		llvm::Value* genSizeOf(Function& function, SEM::Type* type);
		
	}
	
}

#endif
