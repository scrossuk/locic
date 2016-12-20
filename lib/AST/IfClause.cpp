#include <locic/AST/IfClause.hpp>
#include <locic/AST/Scope.hpp>
#include <locic/AST/Value.hpp>

#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		IfClause::IfClause(Node<ValueDecl> pCondition,
		                   Node<Scope> pScope)
		: conditionDecl_(std::move(pCondition)),
		scope_(std::move(pScope)) { }
		
		IfClause::~IfClause() { }
		
		Node<ValueDecl>& IfClause::conditionDecl() {
			return conditionDecl_;
		}
		
		const Node<ValueDecl>& IfClause::conditionDecl() const {
			return conditionDecl_;
		}
		
		Node<Scope>& IfClause::scope() {
			return scope_;
		}
		
		const Node<Scope>& IfClause::scope() const {
			return scope_;
		}
		
		const Value& IfClause::condition() const {
			return condition_;
		}
		
		void IfClause::setCondition(Value pCondition) {
			condition_ = std::move(pCondition);
		}
		
		std::string IfClause::toString() const {
			return makeString("IfClause(condition: %s, scope: %s)",
				conditionDecl()->toString().c_str(),
				scope()->toString().c_str());
		}
		
	}
	
}

