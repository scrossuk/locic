#ifndef LOCIC_CODEGEN_LIVENESS_HPP
#define LOCIC_CODEGEN_LIVENESS_HPP

namespace locic {
	
	namespace AST {
		
		class Type;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class Function;
		class LivenessIndicator;
		class Module;
		
		/**
		 * \brief Get liveness indicator for type.
		 */
		LivenessIndicator getLivenessIndicator(Module& module, const AST::TypeInstance& typeInstance);
		
		/**
		 * \brief Set outer live state.
		 * 
		 * Modifies the object pointer value to be in a live
		 * state; note that this doesn't set any of the member
		 * values into a live state.
		 */
		void setOuterLiveState(Function& function, const AST::TypeInstance& typeInstance, llvm::Value* const objectPointerValue);
		
		/**
		 * \brief Generate set-dead-state.
		 * 
		 * Emits code that modifies the object pointer value to
		 * be in a 'dead' state, meaning that no destructors or
		 * move operations will be performed for it.
		 */
		void genSetDeadState(Function& function, const AST::Type* type, llvm::Value* const objectPointerValue);
		
		/**
		 * \brief Generate set-invalid-state.
		 * 
		 * Emits code that modifies the object pointer value to
		 * be in an 'invalid' state.
		 */
		void genSetInvalidState(Function& function, const AST::Type* type, llvm::Value* const objectPointerValue);
		
		/**
		 * \brief Determine whether an object is live.
		 */
		llvm::Value* genIsLive(Function& function, const AST::Type* type, llvm::Value* value);
		
		/**
		 * \brief Get pointer to liveness byte.
		 */
		llvm::Value* getLivenessBytePtr(Function& function,
		                                const AST::TypeInstance& typeInstance,
		                                LivenessIndicator livenessIndicator,
		                                llvm::Value* objectPointerValue);
		
	}
	
}

#endif
