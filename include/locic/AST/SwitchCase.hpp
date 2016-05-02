#ifndef LOCIC_AST_SWITCHCASE_HPP
#define LOCIC_AST_SWITCHCASE_HPP

#include <vector>

#include <locic/AST/Node.hpp>

namespace locic {

	namespace AST {
	
		struct Scope;
		class Var;
		
		struct SwitchCase {
			Node<Var> var;
			Node<Scope> scope;
			
			SwitchCase(Node<Var> pVar, Node<Scope> pScope)
			: var(std::move(pVar)), scope(std::move(pScope)) { }
		};
		
		typedef std::vector<Node<SwitchCase>> SwitchCaseList;
		
		struct DefaultCase {
			bool hasScope;
			Node<AST::Scope> scope;
			
			static DefaultCase* Empty() {
				return new DefaultCase(false, makeDefaultNode<AST::Scope>());
			}
			
			static DefaultCase* Scope(Node<AST::Scope> scope) {
				return new DefaultCase(true, std::move(scope));
			}
			
			bool isEmpty() const {
				return !hasScope;
			}
			
			DefaultCase(bool pHasScope, Node<AST::Scope> pScope)
			: hasScope(pHasScope), scope(std::move(pScope)) { }
		};
		
	}
	
}

#endif
