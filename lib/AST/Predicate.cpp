#include <stdexcept>
#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/AST/Symbol.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/ValueDecl.hpp>
#include <locic/Support/MakeString.hpp>

namespace locic {
	
	namespace AST {
		
		Predicate* Predicate::True() {
			return new Predicate(TRUE);
		}
		
		Predicate* Predicate::False() {
			return new Predicate(FALSE);
		}
		
		Predicate* Predicate::Bracket(Node<Predicate>expr) {
			Predicate* predicate = new Predicate(BRACKET);
			predicate->bracket_.expr = std::move(expr);
			return predicate;
		}
		
		Predicate* Predicate::TypeSpec(Node<TypeDecl>type, Node<TypeDecl>requireType) {
			Predicate* predicate = new Predicate(TYPESPEC);
			predicate->typeSpec_.type = std::move(type);
			predicate->typeSpec_.requireType = std::move(requireType);
			return predicate;
		}
		
		Predicate* Predicate::Symbol(Node<AST::Symbol>symbol) {
			Predicate* predicate = new Predicate(SYMBOL);
			predicate->symbol_ = std::move(symbol);
			return predicate;
		}
		
		Predicate* Predicate::And(Node<Predicate>left, Node<Predicate>right) {
			Predicate* predicate = new Predicate(AND);
			predicate->and_.left = std::move(left);
			predicate->and_.right = std::move(right);
			return predicate;
		}
		
		Predicate* Predicate::Or(Node<Predicate>left, Node<Predicate>right) {
			Predicate* predicate = new Predicate(OR);
			predicate->or_.left = std::move(left);
			predicate->or_.right = std::move(right);
			return predicate;
		}
		
		Predicate::Predicate(const Kind pKind) : kind_(pKind) { }
		
		Predicate::~Predicate() { }
		
		Predicate::Kind Predicate::kind() const {
			return kind_;
		}
		
		const Node<Predicate>& Predicate::bracketExpr() const {
			assert(kind() == BRACKET);
			return bracket_.expr;
		}
		
		Node<TypeDecl>& Predicate::typeSpecType() {
			assert(kind() == TYPESPEC);
			return typeSpec_.type;
		}
		
		const Node<TypeDecl>& Predicate::typeSpecType() const {
			assert(kind() == TYPESPEC);
			return typeSpec_.type;
		}
		
		Node<TypeDecl>& Predicate::typeSpecRequireType() {
			assert(kind() == TYPESPEC);
			return typeSpec_.requireType;
		}
		
		const Node<TypeDecl>& Predicate::typeSpecRequireType() const {
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
				case TRUE:
					return "True";
				case FALSE:
					return "False";
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
			
			locic_unreachable("Unknown predicate kind.");
		}
		
	}
	
}
