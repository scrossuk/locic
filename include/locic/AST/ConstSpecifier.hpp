#ifndef LOCIC_AST_CONSTSPECIFIER_HPP
#define LOCIC_AST_CONSTSPECIFIER_HPP

#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/PredicateDecl.hpp>

namespace locic {

	namespace AST {
	
		class ConstSpecifier {
			public:
				enum Kind {
					NONE,
					CONST,
					MUTABLE,
					EXPR
				};
				
				static ConstSpecifier* None();
				
				static ConstSpecifier* Const();
				
				static ConstSpecifier* Mutable();
				
				static ConstSpecifier* Expr(Node<PredicateDecl> predicate);
				
				~ConstSpecifier();
				
				Kind kind() const;
				
				bool isNone() const;
				bool isConst() const;
				bool isMutable() const;
				bool isExpr() const;
				
				const Node<PredicateDecl>& predicate() const;
				
			private:
				Kind kind_;
				Node<PredicateDecl> predicate_;
			
				ConstSpecifier(const Kind pKind) : kind_(pKind) { }
		};
		
	}
	
}

#endif
