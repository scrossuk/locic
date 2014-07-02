#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		bool needsLivenessIndicator(Module& module, SEM::Type* type) {
			// Liveness indicator only required if type has
			// destructor (since the liveness indicator is used
			// to determine whether the destructor should be run).
			return typeHasDestructor(module, type);
		}
		
	}
	
}

