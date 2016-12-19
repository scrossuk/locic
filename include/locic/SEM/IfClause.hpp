#ifndef LOCIC_SEM_IFCLAUSE_HPP
#define LOCIC_SEM_IFCLAUSE_HPP

#include <memory>
#include <string>

#include <locic/AST/Value.hpp>

namespace locic {

	namespace SEM {
	
		class Scope;
		
		class IfClause {
			public:
				IfClause(AST::Value condition, std::unique_ptr<Scope> scope);
				
				const AST::Value& condition() const;
				
				Scope& scope() const;
				
				std::string toString() const;
				
			private:
				AST::Value condition_;
				std::unique_ptr<Scope> scope_;
				
		};
		
	}
	
}

#endif
