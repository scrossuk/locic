#ifndef LOCIC_CODEGEN_GENFUNCTION_HPP
#define LOCIC_CODEGEN_GENFUNCTION_HPP

#include <llvm/Function.h>
#include <Locic/SEM.hpp>

#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
		
		llvm::GlobalValue::LinkageTypes getFunctionLinkage(SEM::TypeInstance* parentType);
		
		llvm::Function* genFunction(Module& module, SEM::Type* parent, SEM::Function* function);
		
	}
	
}

#endif
