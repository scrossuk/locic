#ifndef LOCIC_CODEGEN_LIVENESS_HPP
#define LOCIC_CODEGEN_LIVENESS_HPP

namespace locic {
	
	namespace SEM {
		
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class LivenessIndicator;
		class Module;
		
		/**
		 * \brief Query whether a type has a liveness indicator.
		 */
		bool typeInstanceHasLivenessIndicator(const SEM::TypeInstance& typeInstance);
		
		/**
		 * \brief Get liveness indicator for type.
		 */
		LivenessIndicator getLivenessIndicator(Module& module, const SEM::TypeInstance& typeInstance);
		
		/**
		 * \brief Create dead value of given type.
		 */
		llvm::Value* genDeadValue(Function& function, const SEM::Type* type, llvm::Value* const hintResultValue);
		
		/**
		 * \brief Create __dead function definition.
		 */
		llvm::Function* genDeadDefaultFunctionDef(Module& module, const SEM::TypeInstance* const typeInstance);
		
		/**
		 * \brief Determine whether an object is live.
		 */
		llvm::Value* genIsLive(Function& function, const SEM::Type* type, llvm::Value* value);
		
		/**
		 * \brief Create __islive function definition.
		 */
		llvm::Function* genIsLiveDefaultFunctionDef(Module& module, const SEM::TypeInstance* const typeInstance);
		
	}
	
}

#endif
