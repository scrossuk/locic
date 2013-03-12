#ifndef LOCIC_AST_SCOPE_HPP
#define LOCIC_AST_SCOPE_HPP

#include <list>
#include <Locic/AST/Statement.hpp>

namespace AST {

	struct Scope {
		std::vector<Statement*> statements;
		
		inline Scope() { }
		
		inline Scope(const std::vector<Statement*>& s)
			: statements(s) { }
	};
	
}

#endif
