#include <locic/AST/Node.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/AST/RequireSpecifier.hpp>

namespace locic {

	namespace AST {
		
		RequireSpecifier* RequireSpecifier::None() {
			return new RequireSpecifier(NONE);
		}
		
		RequireSpecifier* RequireSpecifier::Expr(const Node<Predicate>& predicate) {
			RequireSpecifier* requireSpecifier = new RequireSpecifier(EXPR);
			requireSpecifier->predicate_ = predicate;
			return requireSpecifier;
		}
		
		RequireSpecifier::Kind RequireSpecifier::kind() const {
			return kind_;
		}
		
		const Node<Predicate>& RequireSpecifier::expr() const {
			assert(kind() == EXPR);
			return predicate_;
		}
		
	}
	
}
