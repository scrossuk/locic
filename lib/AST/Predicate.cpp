#include <stdexcept>
#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/AST/Symbol.hpp>
#include <locic/AST/Type.hpp>
#include <locic/Support/MakeString.hpp>

namespace locic {

	namespace AST {
	
		Predicate* Predicate::Bracket(const Node<Predicate>& expr) {
			Predicate* predicate = new Predicate(BRACKET);
			predicate->bracket_.expr = expr;
			return predicate;
		}
		
		Predicate* Predicate::TypeSpec(const Node<Type>& type, const Node<Type>& requireType) {
			Predicate* predicate = new Predicate(TYPESPEC);
			predicate->typeSpec_.type = type;
			predicate->typeSpec_.requireType = requireType;
			return predicate;
		}
		
		Predicate* Predicate::Symbol(const Node<AST::Symbol>& symbol) {
			Predicate* predicate = new Predicate(SYMBOL);
			predicate->symbol_ = symbol;
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
		
		const Node<Type>& Predicate::typeSpecType() const {
			assert(kind() == TYPESPEC);
			return typeSpec_.type;
		}
		
		const Node<Type>& Predicate::typeSpecRequireType() const {
			assert(kind() == TYPESPEC);
			return typeSpec_.requireType;
		}
		
		const Node<AST::Symbol>& Predicate::symbol() const {
			assert(kind() == SYMBOL);
			return symbol_;
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
		
		std::string Predicate::toString() const {
			switch (kind()) {
				case  BRACKET:
					return makeString("Bracket(%s)", bracketExpr().toString().c_str());
				case TYPESPEC:
					return makeString("TypeSpec(type: %s, require: %s)", typeSpecType().toString().c_str(), typeSpecRequireType().toString().c_str());
				case SYMBOL:
					return makeString("Symbol(symbol: %s)", symbol().toString().c_str());
				case AND:
					return makeString("And(left: %s, right: %s)", andLeft().toString().c_str(), andRight().toString().c_str());
				case OR:
					return makeString("Or(left: %s, right: %s)", orLeft().toString().c_str(), orRight().toString().c_str());
			}
			
			throw std::logic_error("Unknown predicate kind.");
		}
		
	}
	
}
