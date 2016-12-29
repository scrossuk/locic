#ifndef LOCIC_CODEGEN_LIVENESSEMITTER_HPP
#define LOCIC_CODEGEN_LIVENESSEMITTER_HPP

namespace locic {
	
	namespace AST {
		
		class Type;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class IREmitter;
		class LivenessIndicator;
		
		class LivenessEmitter {
		public:
			LivenessEmitter(IREmitter& irEmitter);
			
			llvm::Value*
			emitLivenessByteOffset(const AST::TypeInstance& typeInstance,
			                       LivenessIndicator livenessIndicator);
			
			llvm::Value*
			emitLivenessBytePtr(const AST::TypeInstance& typeInstance,
			                    LivenessIndicator livenessIndicator,
			                    llvm::Value* objectPointerValue);
			
			/**
			 * \brief Set outer live state.
			 * 
			 * Modifies the object pointer value to be in a live
			 * state; note that this doesn't set any of the member
			 * values into a live state.
			 */
			void
			emitSetOuterLive(const AST::TypeInstance& typeInstance,
			                 llvm::Value* objectPointerValue);
			
			/**
			 * \brief Generate set-dead-state.
			 * 
			 * Emits code that modifies the object pointer value to
			 * be in a 'dead' state, meaning that no destructors or
			 * move operations will be performed for it.
			 */
			void
			emitSetDeadCall(const AST::Type* type,
			                llvm::Value* objectPointerValue);
			
			/**
			 * \brief Generate set-invalid-state.
			 * 
			 * Emits code that modifies the object pointer value to
			 * be in an 'invalid' state.
			 */
			void
			emitSetInvalidCall(const AST::Type* type,
			                   llvm::Value* objectPointerValue);
			
			/**
			 * \brief Determine whether an object is live.
			 */
			llvm::Value*
			emitIsLiveCall(const AST::Type* type, llvm::Value* value);
			
		private:
			IREmitter& irEmitter_;
			
		};
		
	}
	
}

#endif
