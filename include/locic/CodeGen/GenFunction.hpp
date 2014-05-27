#ifndef LOCIC_CODEGEN_GENFUNCTION_HPP
#define LOCIC_CODEGEN_GENFUNCTION_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/SEM.hpp>

#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::GlobalValue::LinkageTypes getFunctionLinkage(SEM::TypeInstance* typeInstance, SEM::ModuleScope* moduleScope);
		
		llvm::Function* genFunction(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* function);
		
	}
	
}

#endif
