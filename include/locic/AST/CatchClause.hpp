#ifndef LOCIC_AST_CATCHCLAUSE_HPP
#define LOCIC_AST_CATCHCLAUSE_HPP

#include <vector>

#include <locic/AST/Node.hpp>

namespace locic {

	namespace AST {
	
		class Scope;
		class Var;
		
		struct CatchClause {
			Node<Var> var;
			Node<Scope> scope;
			
			inline CatchClause(Node<Var> pVar, Node<Scope> pScope)
			: var(std::move(pVar)), scope(std::move(pScope)) { }
		};
		
		typedef std::vector<Node<CatchClause>> CatchClauseList;
		
	}
	
}

#endif
