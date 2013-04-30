#include <Locic/String.hpp>

#include <Locic/SEM/Function.hpp>
#include <Locic/SEM/Object.hpp>
#include <Locic/SEM/Scope.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace Locic {

	namespace SEM {
	
		std::string Function::toString() const {
			if (isDeclaration()) {
				return makeString("FunctionDeclaration(name: %s, isMethod: %s, isStatic: %s, type: %s)",
					name().c_str(),
					isMethod() ? "Yes" : "No",
					isStatic() ? "Yes" : "No",
					type()->toString().c_str());
			} else {
				return makeString("FunctionDefinition(name: %s, isMethod: %s, isStatic: %s, type: %s, scope: %s)",
					name().c_str(),
					isMethod() ? "Yes" : "No",
					isStatic() ? "Yes" : "No",
					type()->toString().c_str(),
					scope().toString().c_str());
			}
		}
		
	}
	
}

