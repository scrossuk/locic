#include <map>
#include <string>
#include <vector>

#include <locic/String.hpp>

#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Statement.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Scope::Scope() { }
		
		std::vector<Var*>& Scope::variables() {
			return variables_;
		}
		
		const std::vector<Var*>& Scope::variables() const {
			return variables_;
		}
		
		std::map<std::string, Var*>& Scope::namedVariables() {
			return namedVariables_;
		}
		
		const std::map<std::string, Var*>& Scope::namedVariables() const {
			return namedVariables_;
		}
		
		std::vector<Statement*>& Scope::statements() {
			return statementList_;
		}
		
		const std::vector<Statement*>& Scope::statements() const {
			return statementList_;
		}
		
		std::string Scope::toString() const {
			return makeString("Scope(vars: %s, statements: %s)",
					makeArrayString(variables_).c_str(),
					makeArrayString(statementList_).c_str());
		}
		
	}
	
}

