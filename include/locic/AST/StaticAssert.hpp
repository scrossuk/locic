#ifndef LOCIC_AST_STATICASSERT_HPP
#define LOCIC_AST_STATICASSERT_HPP

#include <string>

#include <locic/AST/Node.hpp>

namespace locic {
	
	namespace AST {
		
		class PredicateDecl;
		
		class StaticAssert {
		public:
			StaticAssert(Node<PredicateDecl> expr);
			
			const Node<PredicateDecl>& expression() const;
			
			std::string toString() const;
			
		private:
			Node<PredicateDecl> expression_;
			
		};
		
	}
	
}

#endif
