#ifndef LOCIC_AST_PREDICATE_HPP
#define LOCIC_AST_PREDICATE_HPP

#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/Symbol.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		struct TypeDecl;
		
		class Predicate {
			public:
				enum Kind {
					TRUE,
					FALSE,
					BRACKET,
					TYPESPEC,
					SYMBOL,
					AND,
					OR
				};
				
				static Predicate* True();
				
				static Predicate* False();
				
				static Predicate* Bracket(Node<Predicate> expr);
				
				static Predicate* TypeSpec(Node<TypeDecl> type, Node<TypeDecl> requireType);
				
				static Predicate* Symbol(Node<AST::Symbol> symbol);
				
				static Predicate* And(Node<Predicate> left, Node<Predicate> right);
				
				static Predicate* Or(Node<Predicate> left, Node<Predicate> right);
				
				Predicate(const Predicate&) = default;
				~Predicate();
				
				Kind kind() const;
				
				const Node<Predicate>& bracketExpr() const;
				
				Node<TypeDecl>& typeSpecType();
				const Node<TypeDecl>& typeSpecType() const;
				Node<TypeDecl>& typeSpecRequireType();
				const Node<TypeDecl>& typeSpecRequireType() const;
				
				const Node<AST::Symbol>& symbol() const;
				
				const Node<Predicate>& andLeft() const;
				const Node<Predicate>& andRight() const;
				
				const Node<Predicate>& orLeft() const;
				const Node<Predicate>& orRight() const;
				
				std::string toString() const;
				
			private:
				Kind kind_;
				
				Node<AST::Symbol> symbol_;
				
				struct {
					Node<Predicate> expr;
				} bracket_;
				
				struct {
					Node<TypeDecl> type;
					Node<TypeDecl> requireType;
				} typeSpec_;
				
				struct {
					Node<Predicate> left;
					Node<Predicate> right;
				} and_;
				
				struct {
					Node<Predicate> left;
					Node<Predicate> right;
				} or_;
			
				Predicate(Kind pKind);
		};
		
	}
	
}

#endif
