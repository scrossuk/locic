#include <stdexcept>
#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/PredicateDecl.hpp>
#include <locic/AST/Symbol.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/ValueDecl.hpp>
#include <locic/Support/MakeString.hpp>

namespace locic {
	
	namespace AST {
		
		PredicateDecl* PredicateDecl::True() {
			return new PredicateDecl(TRUE);
		}
		
		PredicateDecl* PredicateDecl::False() {
			return new PredicateDecl(FALSE);
		}
		
		PredicateDecl* PredicateDecl::SelfConst() {
			return new PredicateDecl(SELFCONST);
		}
		
		PredicateDecl* PredicateDecl::Bracket(Node<PredicateDecl>expr) {
			PredicateDecl* predicate = new PredicateDecl(BRACKET);
			predicate->bracket_.expr = std::move(expr);
			return predicate;
		}
		
		PredicateDecl* PredicateDecl::TypeSpec(Node<TypeDecl>type, Node<TypeDecl>requireType) {
			PredicateDecl* predicate = new PredicateDecl(TYPESPEC);
			predicate->typeSpec_.type = std::move(type);
			predicate->typeSpec_.requireType = std::move(requireType);
			return predicate;
		}
		
		PredicateDecl* PredicateDecl::Symbol(Node<AST::Symbol>symbol) {
			PredicateDecl* predicate = new PredicateDecl(SYMBOL);
			predicate->symbol_ = std::move(symbol);
			return predicate;
		}
		
		PredicateDecl* PredicateDecl::And(Node<PredicateDecl>left, Node<PredicateDecl>right) {
			PredicateDecl* predicate = new PredicateDecl(AND);
			predicate->and_.left = std::move(left);
			predicate->and_.right = std::move(right);
			return predicate;
		}
		
		PredicateDecl* PredicateDecl::Or(Node<PredicateDecl>left, Node<PredicateDecl>right) {
			PredicateDecl* predicate = new PredicateDecl(OR);
			predicate->or_.left = std::move(left);
			predicate->or_.right = std::move(right);
			return predicate;
		}
		
		PredicateDecl::PredicateDecl(const Kind pKind) : kind_(pKind) { }
		
		PredicateDecl::~PredicateDecl() { }
		
		PredicateDecl::Kind PredicateDecl::kind() const {
			return kind_;
		}
		
		const Node<PredicateDecl>& PredicateDecl::bracketExpr() const {
			assert(kind() == BRACKET);
			return bracket_.expr;
		}
		
		Node<TypeDecl>& PredicateDecl::typeSpecType() {
			assert(kind() == TYPESPEC);
			return typeSpec_.type;
		}
		
		const Node<TypeDecl>& PredicateDecl::typeSpecType() const {
			assert(kind() == TYPESPEC);
			return typeSpec_.type;
		}
		
		Node<TypeDecl>& PredicateDecl::typeSpecRequireType() {
			assert(kind() == TYPESPEC);
			return typeSpec_.requireType;
		}
		
		const Node<TypeDecl>& PredicateDecl::typeSpecRequireType() const {
			assert(kind() == TYPESPEC);
			return typeSpec_.requireType;
		}
		
		const Node<AST::Symbol>& PredicateDecl::symbol() const {
			assert(kind() == SYMBOL);
			return symbol_;
		}
		
		const Node<PredicateDecl>& PredicateDecl::andLeft() const {
			assert(kind() == AND);
			return and_.left;
		}
		
		const Node<PredicateDecl>& PredicateDecl::andRight() const {
			assert(kind() == AND);
			return and_.right;
		}
		
		const Node<PredicateDecl>& PredicateDecl::orLeft() const {
			assert(kind() == OR);
			return or_.left;
		}
		
		const Node<PredicateDecl>& PredicateDecl::orRight() const {
			assert(kind() == OR);
			return or_.right;
		}
		
		std::string PredicateDecl::toString() const {
			switch (kind()) {
				case TRUE:
					return "True";
				case FALSE:
					return "False";
				case SELFCONST:
					return "SelfConst";
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
