#ifndef LOCIC_CODEGEN_LIVENESS_HPP
#define LOCIC_CODEGEN_LIVENESS_HPP

namespace locic {
	
	namespace AST {
		
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class LivenessIndicator;
		class Module;
		
		/**
		 * \brief Get liveness indicator for type.
		 */
		LivenessIndicator getLivenessIndicator(Module& module, const AST::TypeInstance& typeInstance);
		
	}
	
}

#endif
