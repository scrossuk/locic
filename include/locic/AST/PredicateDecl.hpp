#ifndef LOCIC_AST_PREDICATEDECL_HPP
#define LOCIC_AST_PREDICATEDECL_HPP

#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/Symbol.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		struct TypeDecl;
		
		class PredicateDecl {
			public:
				enum Kind {
					TRUE,
					FALSE,
					SELFCONST,
					BRACKET,
					TYPESPEC,
					SYMBOL,
					AND,
					OR
				};
				
				static PredicateDecl* True();
				
				static PredicateDecl* False();
				
				static PredicateDecl* SelfConst();
				
				static PredicateDecl* Bracket(Node<PredicateDecl> expr);
				
				static PredicateDecl* TypeSpec(Node<TypeDecl> type, Node<TypeDecl> requireType);
				
				static PredicateDecl* Symbol(Node<AST::Symbol> symbol);
				
				static PredicateDecl* And(Node<PredicateDecl> left, Node<PredicateDecl> right);
				
				static PredicateDecl* Or(Node<PredicateDecl> left, Node<PredicateDecl> right);
				
				PredicateDecl(const PredicateDecl&) = default;
				~PredicateDecl();
				
				Kind kind() const;
				
				const Node<PredicateDecl>& bracketExpr() const;
				
				Node<TypeDecl>& typeSpecType();
				const Node<TypeDecl>& typeSpecType() const;
				Node<TypeDecl>& typeSpecRequireType();
				const Node<TypeDecl>& typeSpecRequireType() const;
				
				const Node<AST::Symbol>& symbol() const;
				
				const Node<PredicateDecl>& andLeft() const;
				const Node<PredicateDecl>& andRight() const;
				
				const Node<PredicateDecl>& orLeft() const;
				const Node<PredicateDecl>& orRight() const;
				
				std::string toString() const;
				
			private:
				Kind kind_;
				
				Node<AST::Symbol> symbol_;
				
				struct {
					Node<PredicateDecl> expr;
				} bracket_;
				
				struct {
					Node<TypeDecl> type;
					Node<TypeDecl> requireType;
				} typeSpec_;
				
				struct {
					Node<PredicateDecl> left;
					Node<PredicateDecl> right;
				} and_;
				
				struct {
					Node<PredicateDecl> left;
					Node<PredicateDecl> right;
				} or_;
			
				PredicateDecl(Kind pKind);
		};
		
	}
	
}

#endif
