#ifndef LOCIC_AST_REQUIRESPECIFIER_HPP
#define LOCIC_AST_REQUIRESPECIFIER_HPP

#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/Type.hpp>

namespace locic {

	namespace AST {
	
		class RequireExpr {
			public:
				enum Kind {
					BRACKET,
					TYPESPEC,
					AND
				};
				
				static RequireExpr* Bracket(const Node<RequireExpr>& expr);
				
				static RequireExpr* TypeSpec(const std::string& name, const Node<Type>& specType);
				
				static RequireExpr* And(const Node<RequireExpr>& left, const Node<RequireExpr>& right);
				
				Kind kind() const;
				
				const Node<RequireExpr>& bracketExpr() const;
				
				const std::string& typeSpecName() const;
				const Node<Type>& typeSpecType() const;
				
				const Node<RequireExpr>& andLeft() const;
				const Node<RequireExpr>& andRight() const;
				
			private:
				Kind kind_;
				
				struct {
					Node<RequireExpr> expr;
				} bracket_;
				
				struct {
					std::string name;
					Node<Type> type;
				} typeSpec_;
				
				struct {
					Node<RequireExpr> left;
					Node<RequireExpr> right;
				} and_;
			
				RequireExpr(Kind pKind) : kind_(pKind) { }
		};
		
		class RequireSpecifier {
			public:
				enum Kind {
					NONE,
					EXPR
				};
				
				static RequireSpecifier* None();
				
				static RequireSpecifier* Expr(const Node<RequireExpr>& expr);
				
				Kind kind() const;
				
				const Node<RequireExpr>& expr() const;
				
			private:
				Kind kind_;
				Node<RequireExpr> expr_;
			
				RequireSpecifier(Kind pKind) : kind_(pKind) { }
		};
		
	}
	
}

#endif
