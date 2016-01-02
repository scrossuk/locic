#ifndef LOCIC_AST_SWITCHCASE_HPP
#define LOCIC_AST_SWITCHCASE_HPP

#include <vector>

#include <locic/AST/Node.hpp>

namespace locic {

	namespace AST {
	
		struct Scope;
		class TypeVar;
		
		struct SwitchCase {
			Node<TypeVar> var;
			Node<Scope> scope;
			
			inline SwitchCase(const Node<TypeVar>& pVar, const Node<Scope>& pScope)
				: var(pVar), scope(pScope) { }
		};
		
		typedef std::vector<Node<SwitchCase>> SwitchCaseList;
		
		struct DefaultCase {
			bool hasScope;
			Node<AST::Scope> scope;
			
			inline static DefaultCase* Empty() {
				return new DefaultCase(false, makeDefaultNode<AST::Scope>());
			}
			
			inline static DefaultCase* Scope(const Node<AST::Scope>& scope) {
				return new DefaultCase(true, scope);
			}
			
			bool isEmpty() const {
				return !hasScope;
			}
			
			inline DefaultCase(bool pHasScope, const Node<AST::Scope>& pScope)
				: hasScope(pHasScope), scope(pScope) { }
		};
		
	}
	
}

#endif
