#ifndef LOCIC_AST_PREDICATE_HPP
#define LOCIC_AST_PREDICATE_HPP

#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/Type.hpp>

namespace locic {

	namespace AST {
	
		class Predicate {
			public:
				enum Kind {
					BRACKET,
					TYPESPEC,
					VARIABLE,
					AND
				};
				
				static Predicate* Bracket(const Node<Predicate>& expr);
				
				static Predicate* TypeSpec(const std::string& name, const Node<Type>& specType);
				
				static Predicate* Variable(const std::string& name);
				
				static Predicate* And(const Node<Predicate>& left, const Node<Predicate>& right);
				
				Kind kind() const;
				
				const Node<Predicate>& bracketExpr() const;
				
				const std::string& typeSpecName() const;
				const Node<Type>& typeSpecType() const;
				
				const std::string& variableName() const;
				
				const Node<Predicate>& andLeft() const;
				const Node<Predicate>& andRight() const;
				
			private:
				Kind kind_;
				
				struct {
					Node<Predicate> expr;
				} bracket_;
				
				struct {
					std::string name;
					Node<Type> type;
				} typeSpec_;
				
				struct {
					std::string name;
				} variable_;
				
				struct {
					Node<Predicate> left;
					Node<Predicate> right;
				} and_;
			
				Predicate(Kind pKind) : kind_(pKind) { }
		};
		
	}
	
}

#endif
