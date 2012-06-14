#ifndef LOCIC_AST_SCOPE_HPP
#define LOCIC_AST_SCOPE_HPP

#include <list>
#include <Locic/AST/Statement.hpp>

namespace AST {

	struct Scope {
		std::list<Statement> statements;
		
		inline Scope(const std::list<Statement>& s)
			: statements(s){ }
	};
	
}

#endif
