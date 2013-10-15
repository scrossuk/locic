#ifndef LOCIC_AST_SCOPE_HPP
#define LOCIC_AST_SCOPE_HPP

#include <list>
#include <Locic/AST/Node.hpp>
#include <Locic/AST/Statement.hpp>

namespace AST {

	struct Scope {
		Node<StatementList> statements;
		
		inline Scope()
			: statements(makeDefaultNode<StatementList>()) { }
		
		inline Scope(const Node<StatementList>& s)
			: statements(s) { }
	};
	
}

#endif
