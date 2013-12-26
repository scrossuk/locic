#ifndef LOCIC_SEM_SCOPE_HPP
#define LOCIC_SEM_SCOPE_HPP

#include <string>
#include <vector>

#include <locic/SEM/Object.hpp>
#include <locic/SEM/Statement.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		class Scope: public Object {
			public:
				inline Scope() { }
				
				inline ObjectKind objectKind() const {
					return OBJECT_SCOPE;
				}
				
				inline std::vector<Var*>& localVariables() {
					return localVariables_;
				}
				
				inline const std::vector<Var*>& localVariables() const {
					return localVariables_;
				}
				
				inline std::vector<Statement*>& statements() {
					return statementList_;
				}
				
				inline const std::vector<Statement*>& statements() const {
					return statementList_;
				}
				
				inline std::string toString() const {
					return makeString("Scope(vars: %s, statements: %s)",
						makeArrayString(localVariables_).c_str(),
						makeArrayString(statementList_).c_str());
				}
				
			private:
				std::vector<Var*> localVariables_;
				std::vector<Statement*> statementList_;
				
		};
		
	}
	
}

#endif
