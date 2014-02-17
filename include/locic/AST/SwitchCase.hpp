#ifndef LOCIC_AST_SWITCHCASE_HPP
#define LOCIC_AST_SWITCHCASE_HPP

#include <vector>

#include <locic/AST/Node.hpp>

namespace locic {

	namespace AST {
	
		struct Scope;
		struct TypeVar;
		
		struct SwitchCase {
			Node<TypeVar> var;
			Node<Scope> scope;
			
			inline SwitchCase(const Node<TypeVar>& pVar, const Node<Scope>& pScope)
				: var(pVar), scope(pScope) { }
		};
		
		typedef std::vector<Node<SwitchCase>> SwitchCaseList;
		
	}
	
}

#endif
