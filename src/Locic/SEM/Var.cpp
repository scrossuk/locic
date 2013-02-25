#include <cstddef>
#include <Locic/String.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>
#include <Locic/SEM/Var.hpp>

namespace Locic {

	namespace SEM {
	
		std::string Var::toString() const {
			return makeString("Var(id: %llu, type: %s, parent: %s)",
					(unsigned long long) id,
					type->toString().c_str(),
					parent != NULL ? parent->toString().c_str() : "[NONE]");
		}
		
	}
	
}

