#ifndef LOCIC_AST_PREDICATE_HPP
#define LOCIC_AST_PREDICATE_HPP

#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/Type.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace AST {
	
		class Predicate {
			public:
				enum Kind {
					BRACKET,
					TYPESPEC,
					VARIABLE,
					AND,
					OR
				};
				
				static Predicate* Bracket(const Node<Predicate>& expr);
				
				static Predicate* TypeSpec(const Node<Type>& type, const Node<Type>& requireType);
				
				static Predicate* Variable(const String& name);
				
				static Predicate* And(const Node<Predicate>& left, const Node<Predicate>& right);
				
				static Predicate* Or(const Node<Predicate>& left, const Node<Predicate>& right);
				
				Kind kind() const;
				
				const Node<Predicate>& bracketExpr() const;
				
				const Node<Type>& typeSpecType() const;
				const Node<Type>& typeSpecRequireType() const;
				
				const String& variableName() const;
				
				const Node<Predicate>& andLeft() const;
				const Node<Predicate>& andRight() const;
				
				const Node<Predicate>& orLeft() const;
				const Node<Predicate>& orRight() const;
				
				std::string toString() const;
				
			private:
				Kind kind_;
				
				struct {
					Node<Predicate> expr;
				} bracket_;
				
				struct {
					Node<Type> type;
					Node<Type> requireType;
				} typeSpec_;
				
				struct {
					String name;
				} variable_;
				
				struct {
					Node<Predicate> left;
					Node<Predicate> right;
				} and_;
				
				struct {
					Node<Predicate> left;
					Node<Predicate> right;
				} or_;
			
				Predicate(Kind pKind) : kind_(pKind) { }
		};
		
	}
	
}

#endif
