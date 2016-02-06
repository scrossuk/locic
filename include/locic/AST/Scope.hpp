#ifndef LOCIC_AST_SCOPE_HPP
#define LOCIC_AST_SCOPE_HPP

#include <list>
#include <locic/AST/Node.hpp>
#include <locic/AST/Statement.hpp>

namespace locic {

	namespace AST {
	
		struct Scope {
			Node<StatementList> statements;
			
			Scope()
			: statements(makeDefaultNode<StatementList>()) { }
				
			Scope(Node<StatementList> s)
			: statements(std::move(s)) { }
			
			size_t size() const {
				return statements->size();
			}
		};
		
	}
	
}

#endif
