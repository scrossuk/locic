#ifndef LOCIC_AST_REQUIRESPECIFIER_HPP
#define LOCIC_AST_REQUIRESPECIFIER_HPP

#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/Predicate.hpp>

namespace locic {

	namespace AST {
		
		class RequireSpecifier {
			public:
				enum Kind {
					NONE,
					NOPREDICATE,
					EXPR
				};
				
				// Not specified.
				static RequireSpecifier* None();
				
				// Specified without a predicate.
				static RequireSpecifier* NoPredicate();
				
				// Specified with a predicate.
				static RequireSpecifier* Expr(Node<Predicate> expr);
				
				Kind kind() const;
				
				bool isNone() const;
				
				bool isNoPredicate() const;
				
				bool isExpr() const;
				
				const Node<Predicate>& expr() const;
				
				std::string toString() const;
				
			private:
				Kind kind_;
				Node<Predicate> predicate_;
			
				RequireSpecifier(const Kind pKind) : kind_(pKind) { }
		};
		
	}
	
}

#endif
