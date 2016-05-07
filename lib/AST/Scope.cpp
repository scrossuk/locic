#include <locic/AST/Node.hpp>
#include <locic/AST/Scope.hpp>
#include <locic/AST/Statement.hpp>

namespace locic {
	
	namespace AST {
		
		Scope::Scope()
		: statements_(makeDefaultNode<StatementList>()) { }
		
		Scope::Scope(Node<StatementList> s)
		: statements_(std::move(s)) { }
		
		size_t Scope::size() const {
			return statements_->size();
		}
		
		Node<StatementList>& Scope::statements() {
			return statements_;
		}
		
		const Node<StatementList>& Scope::statements() const {
			return statements_;
		}
		
	}
	
}

