#ifndef LOCIC_SEM_SCOPE_HPP
#define LOCIC_SEM_SCOPE_HPP

#include <vector>
#include <Locic/SEM/Statement.hpp>
#include <Locic/SEM/Var.hpp>

namespace Locic {

	namespace SEM {
	
		struct Scope {
			std::vector<Var*> localVariables;
			std::vector<Statement*> statementList;
		};
		
	}
	
}

#endif
