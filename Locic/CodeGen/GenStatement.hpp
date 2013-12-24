#ifndef LOCIC_CODEGEN_GENSTATEMENT_HPP
#define LOCIC_CODEGEN_GENSTATEMENT_HPP

#include <Locic/CodeGen/LLVMIncludes.hpp>

#include <Locic/SEM.hpp>

#include <Locic/CodeGen/Function.hpp>

namespace Locic {

	namespace CodeGen {
	
		void genScope(Function& function, const SEM::Scope& scope);
		
		void genStatement(Function& function, SEM::Statement* statement);
		
	}
	
}

#endif
