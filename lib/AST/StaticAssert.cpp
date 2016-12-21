#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/PredicateDecl.hpp>
#include <locic/AST/StaticAssert.hpp>

namespace locic {
	
	namespace AST {
		
		StaticAssert::StaticAssert(Node<PredicateDecl> expr)
		: expression_(std::move(expr)) { }
		
		const Node<PredicateDecl>& StaticAssert::expression() const {
			return expression_;
		}
		
		std::string StaticAssert::toString() const {
			return expression_.toString();
		}
		
	}
	
}

