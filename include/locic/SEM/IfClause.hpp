#ifndef LOCIC_SEM_IFCLAUSE_HPP
#define LOCIC_SEM_IFCLAUSE_HPP

#include <memory>
#include <string>

namespace locic {

	namespace SEM {
	
		class Scope;
		class Value;
		
		class IfClause {
			public:
				IfClause(Value* condition, std::unique_ptr<Scope> scope);
				
				Value* condition() const;
				
				Scope& scope() const;
				
				std::string toString() const;
				
			private:
				Value* condition_;
				std::unique_ptr<Scope> scope_;
				
		};
		
	}
	
}

#endif
