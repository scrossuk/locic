#ifndef LOCIC_AST_CATCHCLAUSE_HPP
#define LOCIC_AST_CATCHCLAUSE_HPP

#include <vector>

#include <locic/AST/Node.hpp>
#include <locic/Support/FastMap.hpp>

namespace locic {

	namespace AST {
	
		class Scope;
		class Var;
		
		class CatchClause {
		public:
			CatchClause(Node<Var> var, Node<Scope> scope);
			
			Node<Var>& var();
			const Node<Var>& var() const;
			
			Node<Scope>& scope();
			const Node<Scope>& scope() const;
			
			FastMap<String, Var*>& namedVariables();
			const FastMap<String, Var*>& namedVariables() const;
			
			std::string toString() const;
			
		private:
			Node<Var> var_;
			Node<Scope> scope_;
			FastMap<String, Var*> namedVariables_;
			
		};
		
		typedef std::vector<Node<CatchClause>> CatchClauseList;
		
	}
	
}

#endif
