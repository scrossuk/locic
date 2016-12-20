#ifndef LOCIC_AST_SWITCHCASE_HPP
#define LOCIC_AST_SWITCHCASE_HPP

#include <vector>

#include <locic/AST/Node.hpp>

#include <locic/Support/FastMap.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace AST {
	
		class Scope;
		class Var;
		
		class SwitchCase {
		public:
			SwitchCase(Node<Var> var, Node<Scope> scope);
			
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
		
		typedef std::vector<Node<SwitchCase>> SwitchCaseList;
		
		class DefaultCase {
		public:
			static DefaultCase* Empty();
			
			static DefaultCase* ScopeCase(Node<Scope> scope);
			
			bool hasScope() const;
			
			Node<Scope>& scope();
			const Node<Scope>& scope() const;
			
		private:
			DefaultCase(Node<Scope> pScope);
			
			Node<Scope> scope_;
		};
		
	}
	
}

#endif
