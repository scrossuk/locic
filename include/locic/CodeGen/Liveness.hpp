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
		bool typeInstanceHasLivenessIndicator(Module& module, const SEM::TypeInstance& typeInstance);
		
		bool typeHasLivenessIndicator(Module& module, const SEM::Type* type);
		
		/**
		 * \brief Get liveness indicator for type.
		 */
		LivenessIndicator getLivenessIndicator(Module& module, const SEM::TypeInstance& typeInstance);
		
		/**
		 * \brief Set outer live state.
		 * 
		 * Modifies the object pointer value to be in a live
		 * state; note that this doesn't set any of the member
		 * values into a live state.
		 */
		void setOuterLiveState(Function& function, const SEM::TypeInstance& typeInstance, llvm::Value* const objectPointerValue);
		
		/**
		 * \brief Generate set-dead-state.
		 * 
		 * Emits code that modifies the object pointer value to
		 * be in a 'dead' state, meaning that no destructors or
		 * move operations will be performed for it.
		 */
		void genSetDeadState(Function& function, const SEM::Type* type, llvm::Value* const objectPointerValue);
		
		/**
		 * \brief Generate set-invalid-state.
		 * 
		 * Emits code that modifies the object pointer value to
		 * be in an 'invalid' state.
		 */
		void genSetInvalidState(Function& function, const SEM::Type* type, llvm::Value* const objectPointerValue);
		
		/**
		 * \brief Create __dead function definition.
		 */
		llvm::Function* genSetDeadDefaultFunctionDef(Module& module, const SEM::TypeInstance* const typeInstance);
		
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
