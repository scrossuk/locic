#ifndef LOCIC_AST_IFCLAUSE_HPP
#define LOCIC_AST_IFCLAUSE_HPP

#include <vector>

#include <locic/AST/Node.hpp>

namespace locic {

	namespace AST {
	
		class Scope;
		struct ValueDecl;
		
		struct IfClause {
			Node<ValueDecl> condition;
			Node<Scope> scope;
			
			IfClause(Node<ValueDecl> pCondition, Node<Scope> pScope)
			: condition(std::move(pCondition)), scope(std::move(pScope)) { }
		};
		
		typedef std::vector<Node<IfClause>> IfClauseList;
		
	}
	
}

#endif
