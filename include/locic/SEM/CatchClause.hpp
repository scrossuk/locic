#ifndef LOCIC_SEM_CATCHCLAUSE_HPP
#define LOCIC_SEM_CATCHCLAUSE_HPP

#include <memory>
#include <string>

#include <locic/FastMap.hpp>
#include <locic/String.hpp>

namespace locic {

	namespace SEM {
	
		class Scope;
		class Var;
		
		class CatchClause {
			public:
				CatchClause();
				
				void setVar(Var* var);
				
				void setScope(std::unique_ptr<Scope> scope);
				
				Var* var() const;
				
				FastMap<String, Var*>& namedVariables();
				const FastMap<String, Var*>& namedVariables() const;
				
				Scope& scope() const;
				
				std::string toString() const;
				
			private:
				Var* var_;
				FastMap<String, Var*> namedVariables_;
				std::unique_ptr<Scope> scope_;
				
		};
		
	}
	
}

#endif
