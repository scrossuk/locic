#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/AST/StaticAssert.hpp>

namespace locic {
	
	namespace AST {
		
		StaticAssert::StaticAssert(Node<Predicate> expr)
		: expression_(std::move(expr)) { }
		
		const Node<Predicate>& StaticAssert::expression() const {
			return expression_;
		}
		
		std::string StaticAssert::toString() const {
			return expression_.toString();
		}
		
	}
	
}

