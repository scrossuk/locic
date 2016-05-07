#ifndef LOCIC_AST_IFCLAUSE_HPP
#define LOCIC_AST_IFCLAUSE_HPP

#include <vector>

#include <locic/AST/Node.hpp>

namespace locic {

	namespace AST {
	
		class Scope;
		struct Value;
		
		struct IfClause {
			Node<Value> condition;
			Node<Scope> scope;
			
			IfClause(Node<Value> pCondition, Node<Scope> pScope)
			: condition(std::move(pCondition)), scope(std::move(pScope)) { }
		};
		
		typedef std::vector<Node<IfClause>> IfClauseList;
		
	}
	
}

#endif
