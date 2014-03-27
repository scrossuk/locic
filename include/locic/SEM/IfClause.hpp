#ifndef LOCIC_SEM_IFCLAUSE_HPP
#define LOCIC_SEM_IFCLAUSE_HPP

#include <string>

namespace locic {

	namespace SEM {
	
		class Scope;
		class Value;
		
		class IfClause {
			public:
				IfClause(Value* condition, Scope* scope);
				
				Value* condition() const;
				
				Scope& scope() const;
				
				std::string toString() const;
				
			private:
				Value* condition_;
				Scope* scope_;
				
		};
		
	}
	
}

#endif
