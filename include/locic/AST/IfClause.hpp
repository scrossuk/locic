#ifndef LOCIC_AST_IFCLAUSE_HPP
#define LOCIC_AST_IFCLAUSE_HPP

#include <vector>

#include <locic/AST/Node.hpp>

namespace locic {

	namespace AST {
	
		struct Scope;
		struct Value;
		
		struct IfClause {
			Node<Value> condition;
			Node<Scope> scope;
			
			inline IfClause(const Node<Value>& pCondition, const Node<Scope>& pScope)
				: condition(pCondition), scope(pScope) { }
		};
		
		typedef std::vector<Node<IfClause>> IfClauseList;
		
	}
	
}

#endif
