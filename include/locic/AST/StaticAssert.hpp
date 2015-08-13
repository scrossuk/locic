#ifndef LOCIC_AST_STATICASSERT_HPP
#define LOCIC_AST_STATICASSERT_HPP

#include <string>

#include <locic/AST/Node.hpp>

namespace locic {
	
	namespace AST {
		
		class Predicate;
		
		class StaticAssert {
		public:
			StaticAssert(const Node<Predicate>& expr);
			
			const Node<Predicate>& expression() const;
			
			std::string toString() const;
			
		private:
			Node<Predicate> expression_;
			
		};
		
	}
	
}

#endif
