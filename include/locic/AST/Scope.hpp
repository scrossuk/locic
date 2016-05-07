#ifndef LOCIC_AST_SCOPE_HPP
#define LOCIC_AST_SCOPE_HPP

#include <locic/AST/Node.hpp>
#include <locic/AST/Statement.hpp>

namespace locic {

	namespace AST {
	
		class Scope {
		public:
			Scope();
			Scope(Node<StatementList> s);
			
			size_t size() const;
			
			Node<StatementList>& statements();
			const Node<StatementList>& statements() const;
			
		private:
			Node<StatementList> statements_;
			
		};
		
	}
	
}

#endif
