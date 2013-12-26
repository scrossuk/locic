#include <locic/String.hpp>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/Object.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {

	namespace SEM {
	
		std::string Function::toString() const {
			if (isDeclaration()) {
				return makeString("FunctionDeclaration(name: %s, isMethod: %s, isStatic: %s, type: %s)",
					name().toString().c_str(),
					isMethod() ? "Yes" : "No",
					isStatic() ? "Yes" : "No",
					type()->toString().c_str());
			} else {
				return makeString("FunctionDefinition(name: %s, isMethod: %s, isStatic: %s, type: %s, scope: %s)",
					name().toString().c_str(),
					isMethod() ? "Yes" : "No",
					isStatic() ? "Yes" : "No",
					type()->toString().c_str(),
					scope().toString().c_str());
			}
		}
		
	}
	
}

