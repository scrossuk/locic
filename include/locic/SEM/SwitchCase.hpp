#ifndef LOCIC_SEM_SWITCHCASE_HPP
#define LOCIC_SEM_SWITCHCASE_HPP

#include <map>
#include <memory>
#include <string>

namespace locic {

	namespace SEM {
	
		class Scope;
		class Var;
		
		class SwitchCase {
			public:
				SwitchCase();
				SwitchCase(Var* var, std::unique_ptr<Scope> scope);
				
				void setVar(Var* var);
				
				void setScope(std::unique_ptr<Scope> scope);
				
				Var* var() const;
				
				std::map<std::string, Var*>& namedVariables();
				const std::map<std::string, Var*>& namedVariables() const;
				
				Scope& scope() const;
				
				std::string toString() const;
				
			private:
				Var* var_;
				std::map<std::string, Var*> namedVariables_;
				std::unique_ptr<Scope> scope_;
				
		};
		
	}
	
}

#endif
