#ifndef LOCIC_CODEGEN_IREMITTER_HPP
#define LOCIC_CODEGEN_IREMITTER_HPP

namespace locic {
	
	class MethodID;
	
	namespace SEM {
		
		class Type;
		
	}
	
	namespace CodeGen {
		
		class ConstantGenerator;
		class Function;
		class Module;
		class PendingResult;
		class TypeGenerator;
		
		/**
		 * \brief LLVM IR Emitter
		 * 
		 * This class provides methods to emit LLVM IR for common
		 * operations.
		 */
		class IREmitter {
		public:
			IREmitter(Function& functionGenerator);
			
			/**
			 * \brief Get constant generator.
			 */
 			ConstantGenerator constantGenerator();
			
			/**
			 * \brief Get type generator.
			 */
 			TypeGenerator typeGenerator();
			
			llvm::BasicBlock*
			createBasicBlock(const char* name = "");
			
			void selectBasicBlock(llvm::BasicBlock* basicBlock);
			
			bool lastInstructionTerminates() const;
			
			void
			emitBranch(llvm::BasicBlock* basicBlock);
			
			void
			emitCondBranch(llvm::Value* condition,
			               llvm::BasicBlock* ifTrue,
			               llvm::BasicBlock* ifFalse);
			
			void
			emitUnreachable();
			
			llvm::Value*
			emitI1ToBool(llvm::Value* value);
			
			llvm::Value*
			emitBoolToI1(llvm::Value* value);
			
			llvm::Value*
			emitRawAlloca(llvm::Type* type);
			
			llvm::Value*
			emitRawLoad(llvm::Value* valuePtr, llvm::Type* type);
			
			void
			emitRawStore(llvm::Value* value, llvm::Value* var);
			
			llvm::Value*
			emitInBoundsGEP(llvm::Type* type,
			                llvm::Value* ptrValue,
			                llvm::Value* indexValue);
			
			llvm::Value*
			emitInBoundsGEP(llvm::Type* type,
			                llvm::Value* ptrValue,
			                llvm::ArrayRef<llvm::Value*> indexArray);
			
			llvm::Value*
			emitConstInBoundsGEP2_32(llvm::Type* type,
			                         llvm::Value* ptrValue,
			                         unsigned index0,
			                         unsigned index1);
			
			llvm::Value*
			emitInsertValue(llvm::Value* aggregate,
			                llvm::Value* value,
			                llvm::ArrayRef<unsigned> indexArray);
			
			void
			emitMemSet(llvm::Value* ptr,
			           llvm::Value* value,
			           uint64_t size,
			           unsigned align);
			
			void
			emitMemSet(llvm::Value* ptr,
			           llvm::Value* value,
			           llvm::Value* sizeValue,
			           unsigned align);
			
			void
			emitMemCpy(llvm::Value* dest,
			           llvm::Value* src,
			           uint64_t size,
			           unsigned align);
			
			void
			emitMemCpy(llvm::Value* dest,
			           llvm::Value* src,
			           llvm::Value* sizeValue,
			           unsigned align);
			
			llvm::CallInst*
			emitCall(llvm::FunctionType* functionType,
			         llvm::Value* callee,
			         llvm::ArrayRef<llvm::Value*> args);
			
			llvm::InvokeInst*
			emitInvoke(llvm::FunctionType* functionType,
			           llvm::Value* callee,
			           llvm::BasicBlock* normalDest,
			           llvm::BasicBlock* unwindDest,
			           llvm::ArrayRef<llvm::Value*> args);
			
			llvm::InvokeInst*
			emitInvoke(llvm::Value* callee,
			           llvm::BasicBlock* normalDest,
			           llvm::BasicBlock* unwindDest,
			           llvm::ArrayRef<llvm::Value*> args);
			
			llvm::ReturnInst*
			emitReturn(llvm::Type* type,
			           llvm::Value* value);
			
			llvm::ReturnInst*
			emitReturnVoid();
			
			llvm::LandingPadInst*
			emitLandingPad(llvm::StructType* type,
			               unsigned numClauses);
			
			llvm::Value*
			emitAlignMask(const SEM::Type* type);
			
			llvm::Value*
			emitSizeOf(const SEM::Type* type);
			
			/**
			 * \brief Create a stack object.
			 * 
			 * This will allocate stack space for the given
			 * type, and return a pointer to that space.
			 */
			llvm::Value*
			emitAlloca(const SEM::Type* type,
			           llvm::Value* const hintResultValue=nullptr);
			
			/**
			 * \brief Move a value by loading it from a memory location.
			 * 
			 * This function can be used to provide move operations; it will
			 * generate a trivial load where possible but may return the
			 * value pointer (as passed to it) if the type has a custom
			 * move method, since this means the object must be kept in memory.
			 */
			llvm::Value*
			emitMoveLoad(llvm::Value* value, const SEM::Type* type);
			
			/**
			 * \brief Move a value by storing it into a memory location.
			 * 
			 * This function can be used to provide move operations; it will
			 * generate a trivial store where possible but will invoke the
			 * move method if the type has a custom move method.
			 */
			void
			emitMoveStore(llvm::Value* value,
			              llvm::Value* memDest,
			              const SEM::Type* type);
			
			/**
			 * \brief Emit a call to a __moveto method.
			 */
			void
			emitMoveCall(llvm::Value* memSource,
			             llvm::Value* memDest,
			             llvm::Value* destOffset,
			             const SEM::Type* type);
			
			/**
			 * \brief Load a value from a memory location.
			 * 
			 * For most primitive types, this function will
			 * generated a load instruction. However, otherwise
			 * this function typically returns the pointer passed
			 * to it as-is, since class types should always be
			 * handled as pointers.
			 */
			llvm::Value*
			emitBasicLoad(llvm::Value* value, const SEM::Type* type);
			
			/**
			 * \brief Store a value into a memory location.
			 * 
			 * As with the load function, this handles both
			 * value types (such as primitives) by generating
			 * a normal store, but also handles reference types
			 * (such as classes) by copying the memory from
			 * one pointer to another.
			 */
			void
			emitBasicStore(llvm::Value* value,
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
			
			void
			emitDestructorCall(llvm::Value* value,
							   const SEM::Type* type);
			
			llvm::Value*
			emitImplicitCopyCall(llvm::Value* valueRef,
			                     const SEM::Type* type,
			                     llvm::Value* hintResultValue = nullptr);
			
			llvm::Value*
			emitExplicitCopyCall(llvm::Value* valueRef,
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
			                const SEM::Type* type);
			
			llvm::Value*
			emitComparisonCall(MethodID methodID,
			                   PendingResult leftValue,
			                   PendingResult rightValue,
			                   const SEM::Type* type);
			
			llvm::Value*
			emitNoArgNoReturnCall(MethodID methodID,
			                      llvm::Value* value,
			                      const SEM::Type* type);
			
			/**
			 * \brief Emit call to empty() method.
			 */
			llvm::Value*
			emitIsEmptyCall(llvm::Value* valueRef,
			                const SEM::Type* type);
			
			/**
			 * \brief Emit call to front() method.
			 */
			llvm::Value*
			emitFrontCall(llvm::Value* valueRef, const SEM::Type* type,
			              const SEM::Type* resultType,
			              llvm::Value* hintResultValue = nullptr);
			
			/**
			 * \brief Emit call to skip_front() method.
			 */
			void
			emitSkipFrontCall(llvm::Value* valueRef,
			                  const SEM::Type* type);
			
			// Needed to support existing code.
			// FIXME: Remove these.
 			llvm::IRBuilder<>& builder();
 			Function& function();
 			Module& module();
			
		private:
			Function& functionGenerator_;
			
		};
		
	}
	
}

#endif
