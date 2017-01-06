#ifndef LOCIC_CODEGEN_IREMITTER_HPP
#define LOCIC_CODEGEN_IREMITTER_HPP

#include <locic/CodeGen/PendingResult.hpp>

namespace locic {
	
	class MethodID;
	
	namespace AST {
		
		class Type;
		
	}
	
	namespace CodeGen {
		
		class ConstantGenerator;
		class Function;
		class Module;
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
			
			llvm::BasicBlock* getBasicBlock();
			
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
			emitPointerCast(llvm::Value* ptr, llvm::Type* type);
			
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
			emitAlignMask(const AST::Type* type);
			
			llvm::Value*
			emitSizeOf(const AST::Type* type);
			
			/**
			 * \brief Create a stack object.
			 * 
			 * This will allocate stack space for the given
			 * type, and return a pointer to that space.
			 */
			llvm::Value*
			emitAlloca(const AST::Type* type,
			           llvm::Value* const hintResultValue=nullptr);
			
			/**
			 * \brief Bind value to address.
			 */
			llvm::Value*
			emitBind(llvm::Value* value, const AST::Type* type);
			
			/**
			 * \brief Load value from a memory location.
			 */
			llvm::Value*
			emitLoad(llvm::Value* ptr, const AST::Type* type);
			
			/**
			 * \brief Store value into a memory location.
			 */
			void
			emitStore(llvm::Value* value, llvm::Value* ptr,
			          const AST::Type* type);
			
			/**
			 * \brief Call __move method with given source/dest.
			 */
			void
			emitMove(llvm::Value* sourcePtr, llvm::Value* destPtr,
			         const AST::Type* type);
			
			/**
			 * \brief Move value into a memory location.
			 */
			void
			emitMoveStore(llvm::Value* value, llvm::Value* ptr,
			              const AST::Type* type);
			
			/**
			 * \brief Emit a call to a __move method.
			 */
			llvm::Value*
			emitMoveCall(PendingResult value, const AST::Type* type,
			             llvm::Value* hintResultValue = nullptr);
			
			/**
			 * \brief Emit a call to an inner __move method.
			 */
			llvm::Value*
			emitInnerMoveCall(llvm::Value* value, const AST::Type* type,
			                  llvm::Value* hintResultValue = nullptr);
			
			llvm::Value*
			emitLoadDatatypeTag(llvm::Value* datatypePtr);
			
			void
			emitStoreDatatypeTag(llvm::Value* tagValue,
			                     llvm::Value* datatypePtr);
			
			llvm::Value*
			emitGetDatatypeVariantPtr(llvm::Value* datatypePtr,
			                          const AST::Type* datatypeType,
			                          const AST::Type* variantType);
			
			llvm::Value*
			emitConstructorCall(const AST::Type* type,
			                    PendingResultArray args,
			                    llvm::Value* hintResultValue = nullptr);
			
			void
			emitDestructorCall(llvm::Value* value,
			                   const AST::Type* type);
			
			void
			emitInnerDestructorCall(llvm::Value* value,
			                        const AST::Type* type);
			
			void
			scheduleDestructorCall(llvm::Value* value,
			                       const AST::Type* type);
			
			llvm::Value*
			emitImplicitCopyCall(llvm::Value* valueRef,
			                     const AST::Type* type,
			                     llvm::Value* hintResultValue = nullptr);
			
			llvm::Value*
			emitExplicitCopyCall(llvm::Value* valueRef,
			                     const AST::Type* type,
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
			             const AST::Type* type,
			             llvm::Value* hintResultValue = nullptr);
			
			llvm::Value*
			emitCompareCall(llvm::Value* leftValue,
			                llvm::Value* rightValue,
			                const AST::Type* type);
			
			llvm::Value*
			emitComparisonCall(MethodID methodID,
			                   PendingResult leftValue,
			                   PendingResult rightValue,
			                   const AST::Type* type);
			
			llvm::Value*
			emitNoArgNoReturnCall(MethodID methodID,
			                      llvm::Value* value,
			                      const AST::Type* type);
			
			/**
			 * \brief Emit call to empty() method.
			 */
			llvm::Value*
			emitIsEmptyCall(llvm::Value* valueRef,
			                const AST::Type* type);
			
			/**
			 * \brief Emit call to front() method.
			 */
			llvm::Value*
			emitFrontCall(llvm::Value* valueRef, const AST::Type* type,
			              const AST::Type* resultType,
			              llvm::Value* hintResultValue = nullptr);
			
			/**
			 * \brief Emit call to skip_front() method.
			 */
			void
			emitSkipFrontCall(llvm::Value* valueRef,
			                  const AST::Type* type);
			
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
