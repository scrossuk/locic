#include <stdexcept>

#include <locic/AST/Node.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/Support/MakeString.hpp>

namespace locic {

	namespace AST {
		
		RequireSpecifier* RequireSpecifier::None() {
			return new RequireSpecifier(NONE);
		}
		
		RequireSpecifier* RequireSpecifier::NoPredicate() {
			return new RequireSpecifier(NOPREDICATE);
		}
		
		RequireSpecifier* RequireSpecifier::Expr(const Node<Predicate>& predicate) {
			RequireSpecifier* requireSpecifier = new RequireSpecifier(EXPR);
			requireSpecifier->predicate_ = predicate;
			return requireSpecifier;
		}
		
		RequireSpecifier::Kind RequireSpecifier::kind() const {
			return kind_;
		}
		
		bool RequireSpecifier::isNone() const {
			return kind() == NONE;
		}
		
		bool RequireSpecifier::isNoPredicate() const {
			return kind() == NOPREDICATE;
		}
		
		bool RequireSpecifier::isExpr() const {
			return kind() == EXPR;
		}
		
		const Node<Predicate>& RequireSpecifier::expr() const {
			assert(kind() == EXPR);
			return predicate_;
		}
		
		std::string RequireSpecifier::toString() const {
			switch (kind()) {
				case NONE:
					return "RequireSpecifier(kind = NONE)";
				case NOPREDICATE:
					return "RequireSpecifier(kind = NOPREDICATE)";
				case EXPR:
					return makeString("RequireSpecifier(kind = EXPR, expr = %s)", expr().toString().c_str());
			}
			
			throw std::logic_error("Unknown require specifier kind.");
		}
		
	}
	
}
