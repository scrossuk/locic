#ifndef LOCIC_CODEGEN_IREMITTER_HPP
#define LOCIC_CODEGEN_IREMITTER_HPP

namespace locic {
	
	class MethodID;
	
	namespace SEM {
		
		class Type;
		
	}
	
	namespace CodeGen {
		
		class Function;
		
		/**
		 * \brief LLVM IR Emitter
		 * 
		 * This class provides methods to emit LLVM IR for common
		 * operations.
		 */
		class IREmitter {
		public:
			IREmitter(Function& functionGenerator);
			
			llvm::Value*
			emitAlignMask(const SEM::Type* type);
			
			llvm::Value*
			emitSizeOf(const SEM::Type* type);
			
			llvm::Value*
			emitAlloca(const SEM::Type* type,
			           llvm::Value* hintResultValue = nullptr);
			
			llvm::Value*
			emitMoveLoad(llvm::Value* value, const SEM::Type* type);
			
			void
			emitMoveStore(llvm::Value* value,
			              llvm::Value* memDest,
			              const SEM::Type* type);
			
			llvm::Value*
			emitLoadDatatypeTag(llvm::Value* datatypePtr);
			
			void
			emitStoreDatatypeTag(llvm::Value* tagValue,
			                     llvm::Value* datatypePtr);
			
			llvm::Value*
			emitGetDatatypeVariantPtr(llvm::Value* datatypePtr,
			                          const SEM::Type* datatypeType,
			                          const SEM::Type* variantType);
			
			llvm::Value*
			emitImplicitCopyCall(llvm::Value* value,
			                     const SEM::Type* type,
			                     llvm::Value* hintResultValue = nullptr);
			
			llvm::Value*
			emitExplicitCopyCall(llvm::Value* value,
			                     const SEM::Type* type,
			                     llvm::Value* hintResultValue = nullptr);
			
			/**
			 * \brief Emit call to copy method.
			 * 
			 * This function can emit either a call to implicit copy
			 * or explicit copy methods (specified via MethodID).
			 * 
			 * \param methodID Either METHOD_IMPLICITCOPY or METHOD_COPY.
			 * \param valueRef A reference (i.e. IR pointer) to the value to be copied.
			 * \param type The type of the value to be copied.
			 * \param hintResultValue If given, a pointer to memory where
			 *                        the result **can** be placed, to
			 *                        avoid generating unnecessary allocas.
			 * \return The result of the copy operation.
			 */
			llvm::Value*
			emitCopyCall(MethodID methodID,
			             llvm::Value* valueRef,
			             const SEM::Type* type,
			             llvm::Value* hintResultValue = nullptr);
			
			llvm::Value*
			emitCompareCall(llvm::Value* leftValue,
			                llvm::Value* rightValue,
			                const SEM::Type* compareResultType,
			                const SEM::Type* thisType,
			                const SEM::Type* thisRefType);
			
		private:
			Function& functionGenerator_;
			
		};
		
	}
	
}

#endif
