#ifndef LOCIC_CODEGEN_IREMITTER_HPP
#define LOCIC_CODEGEN_IREMITTER_HPP

#include <llvm-abi/Type.hpp>

#include <locic/CodeGen/PendingResult.hpp>
#include <locic/CodeGen/UnwindState.hpp>

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
			getUndef(llvm_abi::Type type);
			
			llvm::Value*
			emitPointerCast(llvm::Value* ptr, llvm::Type* type);
			
			llvm::Value*
			emitI1ToBool(llvm::Value* value);
			
			llvm::Value*
			emitBoolToI1(llvm::Value* value);
			
			llvm::Value*
			emitRawAlloca(llvm_abi::Type type,
			              llvm::Value* arraySize = nullptr,
			              const llvm::Twine& name = "");
			
			llvm::Value*
			emitRawLoad(llvm::Value* valuePtr, llvm_abi::Type type);
			
			void
			emitRawStore(llvm::Value* value, llvm::Value* var);
			
			llvm::Value*
			emitInBoundsGEP(llvm_abi::Type type,
			                llvm::Value* ptrValue,
			                llvm::Value* indexValue);
			
			llvm::Value*
			emitInBoundsGEP(llvm_abi::Type type,
			                llvm::Value* ptrValue,
			                llvm::ArrayRef<llvm::Value*> indexArray);
			
			llvm::Value*
			emitConstInBoundsGEP2_32(llvm_abi::Type type,
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
			
			/**
			 * \brief Emit code to return void.
			 * 
			 * This is a convenience method for calling
			 * emitRawReturn() with a void value.
			 * 
			 * WARNING: This method does not perform unwinding; it
			 *          assumes there are no unwind actions (e.g.
			 *          destructors) to be executed.
			 */
			void
			emitRawReturnVoid();
			
			/**
			 * \brief Emit code to return given value.
			 * 
			 * This method emits code that returns the value,
			 * performing ABI-encoding as necessary.
			 * 
			 * WARNING: This method does not perform unwinding; it
			 *          assumes there are no unwind actions (e.g.
			 *          destructors) to be executed.
			 */
			void
			emitRawReturn(llvm::Value* value);
			
			/**
			 * \brief Emit unwind actions for given state.
			 * 
			 * This method emits code for all current unwind actions
			 * relevant to the given state.
			 */
			void
			emitUnwind(UnwindState unwindState);
			
			/**
			 * \brief Emit code to unwind and return void.
			 * 
			 * This is a convenience method for calling emitReturn()
			 * with a void value.
			 */
			void
			emitReturnVoid();
			
			/**
			 * \brief Emit code to unwind and return given value.
			 * 
			 * This method emits code that executes all unwind
			 * actions and returns the value, performing
			 * ABI-encoding as necessary.
			 */
			void
			emitReturn(llvm::Value* value);
			
			/**
			 * \brief Emit code to load return value after unwind.
			 * 
			 * Return values are saved to the stack while unwind
			 * actions are executed. This method loads the value
			 * once unwinding is complete.
			 */
			llvm::Value*
			emitUnwindLoadReturnValue();
			
			/**
			 * \brief Emit code to store return value before unwind.
			 * 
			 * Return values are saved to the stack while unwind
			 * actions are executed. This method stores the value
			 * before executing unwind actions.
			 */
			void
			emitUnwindSaveReturnValue(llvm::Value* value);
			
			llvm::LandingPadInst*
			emitLandingPad(llvm::StructType* type,
			               unsigned numClauses);
			
			llvm::Value*
			emitAlignMask(const AST::Type* type);
			
			llvm::Value*
			emitSizeOf(const AST::Type* type);
			
			/**
			 * \brief Create uninitialised stack object.
			 * 
			 * This will allocate stack space for the given
			 * type, and return a pointer to that space.
			 */
			llvm::Value*
			emitUninitialisedAlloca(const AST::Type* type,
			                        llvm::Value* resultPtr,
			                        const llvm::Twine& name = "");
			
			/**
			 * \brief Create a stack object.
			 * 
			 * This will allocate stack space for the given
			 * type, and return a pointer to that space.
			 * 
			 * Unless the compiler has been requested to always
			 * zero allocas, this will return an identical result to
			 * emitUninitialisedAlloca().
			 */
			llvm::Value*
			emitAlloca(const AST::Type* type,
			           llvm::Value* const resultPtr = nullptr,
			           const llvm::Twine& name = "");
			
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
			             llvm::Value* resultPtr = nullptr);
			
			/**
			 * \brief Emit a call to an inner __move method.
			 */
			llvm::Value*
			emitInnerMoveCall(llvm::Value* value, const AST::Type* type,
			                  llvm::Value* resultPtr = nullptr);
			
			llvm::Value*
			emitLoadVariantTag(llvm::Value* datatypePtr);
			
			void
			emitStoreVariantTag(llvm::Value* tagValue,
			                    llvm::Value* datatypePtr);
			
			llvm::Value*
			emitGetVariantValuePtr(llvm::Value* variantPtr,
			                       const AST::Type* variantType);
			
			llvm::Value*
			emitConstructorCall(const AST::Type* type,
			                    PendingResultArray args,
			                    llvm::Value* resultPtr = nullptr);
			
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
			                     llvm::Value* resultPtr = nullptr);
			
			llvm::Value*
			emitExplicitCopyCall(llvm::Value* valueRef,
			                     const AST::Type* type,
			                     llvm::Value* resultPtr = nullptr);
			
			/**
			 * \brief Emit call to copy method.
			 * 
			 * This function can emit either a call to implicit copy
			 * or explicit copy methods (specified via MethodID).
			 * 
			 * \param methodID Either METHOD_IMPLICITCOPY or METHOD_COPY.
			 * \param valueRef A reference (i.e. IR pointer) to the value to be copied.
			 * \param type The type of the value to be copied.
			 * \param resultPtr If given, a pointer to memory for the result (if type
			 *                  is NOT passed by value).
			 * \return The result of the copy operation.
			 */
			llvm::Value*
			emitCopyCall(MethodID methodID,
			             llvm::Value* valueRef,
			             const AST::Type* type,
			             llvm::Value* resultPtr = nullptr);
			
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
			              llvm::Value* resultPtr = nullptr);
			
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
