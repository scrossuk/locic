#ifndef LOCIC_SEM_CATCHCLAUSE_HPP
#define LOCIC_SEM_CATCHCLAUSE_HPP

#include <map>
#include <string>

namespace locic {

	namespace SEM {
	
		class Scope;
		class Var;
		
		class CatchClause {
			public:
				CatchClause();
				
				void setVar(Var* var);
				
				void setScope(Scope* scope);
				
				Var* var() const;
				
				std::map<std::string, Var*>& namedVariables();
				const std::map<std::string, Var*>& namedVariables() const;
				
				Scope& scope() const;
				
				std::string toString() const;
				
			private:
				Var* var_;
				std::map<std::string, Var*> namedVariables_;
				Scope* scope_;
				
		};
		
	}
	
}

#endif
