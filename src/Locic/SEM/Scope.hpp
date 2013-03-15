#ifndef LOCIC_SEM_SCOPE_HPP
#define LOCIC_SEM_SCOPE_HPP

#include <vector>

#include <Locic/SEM/Object.hpp>
#include <Locic/SEM/Statement.hpp>
#include <Locic/SEM/Var.hpp>

namespace Locic {

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
				
			private:
				std::vector<Var*> localVariables_;
				std::vector<Statement*> statementList_;
				
		};
		
	}
	
}

#endif
