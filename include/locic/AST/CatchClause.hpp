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
			
			inline CatchClause(const Node<TypeVar>& pVar, const Node<Scope>& pScope)
				: var(pVar), scope(pScope) { }
		};
		
		typedef std::vector<Node<CatchClause>> CatchClauseList;
		
	}
	
}

#endif
