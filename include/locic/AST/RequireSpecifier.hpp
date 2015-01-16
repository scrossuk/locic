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
					EXPR
				};
				
				static RequireSpecifier* None();
				
				static RequireSpecifier* Expr(const Node<Predicate>& expr);
				
				Kind kind() const;
				
				const Node<Predicate>& expr() const;
				
			private:
				Kind kind_;
				Node<Predicate> predicate_;
			
				RequireSpecifier(const Kind pKind) : kind_(pKind) { }
		};
		
	}
	
}

#endif
