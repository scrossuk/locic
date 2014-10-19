#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/Type.hpp>

namespace locic {

	namespace AST {
	
		RequireExpr* RequireExpr::Bracket(const Node<RequireExpr>& expr) {
			RequireExpr* requireExpr = new RequireExpr(BRACKET);
			requireExpr->bracket_.expr = expr;
			return requireExpr;
		}
		
		RequireExpr* RequireExpr::TypeSpec(const std::string& name, const Node<Type>& specType) {
			RequireExpr* requireExpr = new RequireExpr(TYPESPEC);
			requireExpr->typeSpec_.name = name;
			requireExpr->typeSpec_.type = specType;
			return requireExpr;
		}
		
		RequireExpr* RequireExpr::And(const Node<RequireExpr>& left, const Node<RequireExpr>& right) {
			RequireExpr* requireExpr = new RequireExpr(AND);
			requireExpr->and_.left = left;
			requireExpr->and_.right = right;
			return requireExpr;
		}
		
		RequireExpr::Kind RequireExpr::kind() const {
			return kind_;
		}
		
		const Node<RequireExpr>& RequireExpr::bracketExpr() const {
			assert(kind() == BRACKET);
			return bracket_.expr;
		}
		
		const std::string& RequireExpr::typeSpecName() const {
			assert(kind() == TYPESPEC);
			return typeSpec_.name;
		}
		
		const Node<Type>& RequireExpr::typeSpecType() const {
			assert(kind() == TYPESPEC);
			return typeSpec_.type;
		}
		
		const Node<RequireExpr>& RequireExpr::andLeft() const {
			assert(kind() == AND);
			return and_.left;
		}
		
		const Node<RequireExpr>& RequireExpr::andRight() const {
			assert(kind() == AND);
			return and_.right;
		}
		
		RequireSpecifier* RequireSpecifier::None() {
			return new RequireSpecifier(NONE);
		}
		
		RequireSpecifier* RequireSpecifier::Expr(const Node<RequireExpr>& expr) {
			RequireSpecifier* requireSpecifier = new RequireSpecifier(EXPR);
			requireSpecifier->expr_ = expr;
			return requireSpecifier;
		}
		
		RequireSpecifier::Kind RequireSpecifier::kind() const {
			return kind_;
		}
		
		const Node<RequireExpr>& RequireSpecifier::expr() const {
			assert(kind() == EXPR);
			return expr_;
		}
		
	}
	
}
