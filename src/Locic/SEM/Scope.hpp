#ifndef LOCIC_SEM_SCOPE_HPP
#define LOCIC_SEM_SCOPE_HPP

#include <list>
#include <vector>
#include <Locic/SEM/Statement.hpp>
#include <Locic/SEM/Var.hpp>

namespace SEM{

	struct Scope{
		std::vector<Var *> localVariables;
		std::list<Statement *> statementList;
	};

}

#endif
