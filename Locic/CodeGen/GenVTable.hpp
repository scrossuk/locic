#ifndef LOCIC_CODEGEN_GENVTABLE_HPP
#define LOCIC_CODEGEN_GENVTABLE_HPP

#include <Locic/SEM.hpp>
#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
	
		llvm::GlobalVariable* genVTable(Module& module, SEM::Type* type);
		
	}
	
}

#endif
