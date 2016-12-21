#ifndef LOCIC_AST_REQUIRESPECIFIER_HPP
#define LOCIC_AST_REQUIRESPECIFIER_HPP

#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/PredicateDecl.hpp>

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
				static RequireSpecifier* Expr(Node<PredicateDecl> expr);
				
				Kind kind() const;
				
				bool isNone() const;
				
				bool isNoPredicate() const;
				
				bool isExpr() const;
				
				const Node<PredicateDecl>& expr() const;
				
				std::string toString() const;
				
			private:
				Kind kind_;
				Node<PredicateDecl> predicate_;
			
				RequireSpecifier(const Kind pKind) : kind_(pKind) { }
		};
		
	}
	
}

#endif
