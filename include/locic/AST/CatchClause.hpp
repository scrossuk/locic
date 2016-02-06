#ifndef LOCIC_AST_CATCHCLAUSE_HPP
#define LOCIC_AST_CATCHCLAUSE_HPP

#include <vector>

#include <locic/AST/Node.hpp>

namespace locic {

	namespace AST {
	
		struct Scope;
		class TypeVar;
		
		struct CatchClause {
			Node<TypeVar> var;
			Node<Scope> scope;
			
			inline CatchClause(Node<TypeVar> pVar, Node<Scope> pScope)
			: var(std::move(pVar)), scope(std::move(pScope)) { }
		};
		
		typedef std::vector<Node<CatchClause>> CatchClauseList;
		
	}
	
}

#endif
