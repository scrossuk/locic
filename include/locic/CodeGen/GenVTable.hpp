#ifndef LOCIC_CODEGEN_GENVTABLE_HPP
#define LOCIC_CODEGEN_GENVTABLE_HPP

#include <locic/SEM.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Value* genVTable(Module& module, const AST::TypeInstance* typeInstance);
		
	}
	
}

#endif
