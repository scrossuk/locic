#ifndef LOCIC_SEM_IFCLAUSE_HPP
#define LOCIC_SEM_IFCLAUSE_HPP

#include <memory>
#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/Value.hpp>

namespace locic {
	
	namespace AST {
		
		class Scope;
		
	}
	
	namespace SEM {
		
		class IfClause {
			public:
				IfClause(AST::Value condition,
				         AST::Node<AST::Scope> scope);
				
				const AST::Value& condition() const;
				
				AST::Scope& scope() const;
				
				std::string toString() const;
				
			private:
				AST::Value condition_;
				AST::Node<AST::Scope> scope_;
				
		};
		
	}
	
}

#endif
