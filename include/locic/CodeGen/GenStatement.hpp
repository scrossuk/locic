#ifndef LOCIC_CODEGEN_GENSTATEMENT_HPP
#define LOCIC_CODEGEN_GENSTATEMENT_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/Function.hpp>

namespace locic {

	namespace CodeGen {
	
		void genScope(Function& function, const SEM::Scope& scope);
		
		void genStatement(Function& function, SEM::Statement* statement);
		
	}
	
}

#endif
