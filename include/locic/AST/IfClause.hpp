#ifndef LOCIC_AST_IFCLAUSE_HPP
#define LOCIC_AST_IFCLAUSE_HPP

#include <string>
#include <vector>

#include <locic/AST/Node.hpp>
#include <locic/AST/Value.hpp>

namespace locic {

	namespace AST {
	
		class Scope;
		struct ValueDecl;
		
		class IfClause {
		public:
			IfClause(Node<ValueDecl> condition,
			         Node<Scope> scope);
			~IfClause();
			
			Node<ValueDecl>& conditionDecl();
			const Node<ValueDecl>& conditionDecl() const;
			
			Node<Scope>& scope();
			const Node<Scope>& scope() const;
			
			const Value& condition() const;
			void setCondition(Value value);
			
			std::string toString() const;
			
		private:
			Node<ValueDecl> conditionDecl_;
			Node<Scope> scope_;
			Value condition_;
			
		};
		
		typedef std::vector<Node<IfClause>> IfClauseList;
		
	}
	
}

#endif
