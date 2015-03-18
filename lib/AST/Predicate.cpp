#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/AST/Type.hpp>

namespace locic {

	namespace AST {
	
		Predicate* Predicate::Bracket(const Node<Predicate>& expr) {
			Predicate* predicate = new Predicate(BRACKET);
			predicate->bracket_.expr = expr;
			return predicate;
		}
		
		Predicate* Predicate::TypeSpec(const String& name, const Node<Type>& specType) {
			Predicate* predicate = new Predicate(TYPESPEC);
			predicate->typeSpec_.name = name;
			predicate->typeSpec_.type = specType;
			return predicate;
		}
		
		Predicate* Predicate::Variable(const String& name) {
			Predicate* predicate = new Predicate(VARIABLE);
			predicate->variable_.name = name;
			return predicate;
		}
		
		Predicate* Predicate::And(const Node<Predicate>& left, const Node<Predicate>& right) {
			Predicate* predicate = new Predicate(AND);
			predicate->and_.left = left;
			predicate->and_.right = right;
			return predicate;
		}
		
		Predicate* Predicate::Or(const Node<Predicate>& left, const Node<Predicate>& right) {
			Predicate* predicate = new Predicate(OR);
			predicate->or_.left = left;
			predicate->or_.right = right;
			return predicate;
		}
		
		Predicate::Kind Predicate::kind() const {
			return kind_;
		}
		
		const Node<Predicate>& Predicate::bracketExpr() const {
			assert(kind() == BRACKET);
			return bracket_.expr;
		}
		
		const String& Predicate::typeSpecName() const {
			assert(kind() == TYPESPEC);
			return typeSpec_.name;
		}
		
		const Node<Type>& Predicate::typeSpecType() const {
			assert(kind() == TYPESPEC);
			return typeSpec_.type;
		}
		
		const String& Predicate::variableName() const {
			assert(kind() == VARIABLE);
			return variable_.name;
		}
		
		const Node<Predicate>& Predicate::andLeft() const {
			assert(kind() == AND);
			return and_.left;
		}
		
		const Node<Predicate>& Predicate::andRight() const {
			assert(kind() == AND);
			return and_.right;
		}
		
		const Node<Predicate>& Predicate::orLeft() const {
			assert(kind() == OR);
			return or_.left;
		}
		
		const Node<Predicate>& Predicate::orRight() const {
			assert(kind() == OR);
			return or_.right;
		}
		
	}
	
}
