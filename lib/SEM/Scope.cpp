#include <string>
#include <vector>

#include <locic/String.hpp>

#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Statement.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Scope::Scope() { }
		
		std::vector<Var*>& Scope::localVariables() {
			return localVariables_;
		}
		
		const std::vector<Var*>& Scope::localVariables() const {
			return localVariables_;
		}
		
		std::vector<Statement*>& Scope::statements() {
			return statementList_;
		}
		
		const std::vector<Statement*>& Scope::statements() const {
			return statementList_;
		}
		
		std::string Scope::toString() const {
			return makeString("Scope(vars: %s, statements: %s)",
							  makeArrayString(localVariables_).c_str(),
							  makeArrayString(statementList_).c_str());
		}
		
	}
	
}

