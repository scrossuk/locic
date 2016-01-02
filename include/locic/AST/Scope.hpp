#ifndef LOCIC_AST_SCOPE_HPP
#define LOCIC_AST_SCOPE_HPP

#include <list>
#include <locic/AST/Node.hpp>
#include <locic/AST/Statement.hpp>

namespace locic {

	namespace AST {
	
		struct Scope {
			Node<StatementList> statements;
			
			inline Scope()
				: statements(makeDefaultNode<StatementList>()) { }
				
			inline Scope(const Node<StatementList>& s)
				: statements(s) { }
			
			size_t size() const {
				return statements->size();
			}
		};
		
	}
	
}

#endif
