#ifndef LOCIC_CODEGEN_GENFUNCTION_HPP
#define LOCIC_CODEGEN_GENFUNCTION_HPP

#include <vector>

#include <Locic/SEM.hpp>

#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
		
		llvm::Function* genFunction(Module& module, SEM::Type* parent, SEM::Function* function);
		
	}
	
}

#endif
