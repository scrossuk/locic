#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>

namespace locic {

	namespace CodeGen {
	
		bool needsLivenessIndicator(Module& module, const SEM::Type* const type) {
			// Liveness indicator only required if type has a destructor
			// or a custom move method, since the liveness indicator is used
			// to determine whether the destructor or move method should be
			// called.
			return typeHasDestructor(module, type) || typeHasCustomMove(module, type);
		}
		
	}
	
}

