#include <locic/AST/ConstSpecifier.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/PredicateDecl.hpp>

namespace locic {

	namespace AST {
		
		ConstSpecifier* ConstSpecifier::None() {
			return new ConstSpecifier(NONE);
		}
		
		ConstSpecifier* ConstSpecifier::Const() {
			return new ConstSpecifier(CONST);
		}
		
		ConstSpecifier* ConstSpecifier::Mutable() {
			return new ConstSpecifier(MUTABLE);
		}
		
		ConstSpecifier* ConstSpecifier::Expr(Node<PredicateDecl> predicate) {
			const auto constSpecifier = new ConstSpecifier(EXPR);
			constSpecifier->predicate_ = std::move(predicate);
			return constSpecifier;
		}
		
		ConstSpecifier::~ConstSpecifier() { }
		
		ConstSpecifier::Kind ConstSpecifier::kind() const {
			return kind_;
		}
		
		bool ConstSpecifier::isNone() const {
			return kind() == NONE;
		}
		
		bool ConstSpecifier::isConst() const {
			return kind() == CONST;
		}
		
		bool ConstSpecifier::isMutable() const {
			return kind() == MUTABLE;
		}
		
		bool ConstSpecifier::isExpr() const {
			return kind() == EXPR;
		}
		
		const Node<PredicateDecl>& ConstSpecifier::predicate() const {
			assert(kind() == EXPR);
			return predicate_;;
		}
		
	}
	
}
