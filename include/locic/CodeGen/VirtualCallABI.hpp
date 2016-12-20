#ifndef LOCIC_CODEGEN_VIRTUALCALLABI_HPP
#define LOCIC_CODEGEN_VIRTUALCALLABI_HPP

namespace locic {
	
	namespace AST {
		
		class Function;
		class FunctionType;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class IREmitter;
		struct VirtualMethodComponents;
		
		class VirtualCallABI {
		public:
			virtual ~VirtualCallABI() { }
			
			/**
			 * \brief Emit vtable slot for a set of conflicting methods.
			 * 
			 * Produces a vtable slot that can call the given array of
			 * functions; if there is more than one method then this
			 * will have to emit a conflict resolution stub.
			 */
			virtual llvm::Constant*
			emitVTableSlot(const AST::TypeInstance& typeInstance,
			               llvm::ArrayRef<AST::Function*> methods) = 0;
			
			/**
			 * \brief Emit virtual call.
			 */
			virtual llvm::Value*
			emitCall(IREmitter& irEmitter,
			         AST::FunctionType functionType,
			         VirtualMethodComponents methodComponents,
			         llvm::ArrayRef<llvm::Value*> args,
			         llvm::Value* hintResultValue) = 0;
			
			enum CountFnKind {
				ALIGNOF,
				SIZEOF
			};
			
			virtual llvm::Value*
			emitCountFnCall(IREmitter& irEmitter,
			                llvm::Value* typeInfoValue,
			                CountFnKind kind) = 0;
			
			virtual void
			emitMoveCall(IREmitter& irEmitter,
			             llvm::Value* typeInfoValue,
			             llvm::Value* sourceValue,
			             llvm::Value* destValue,
			             llvm::Value* positionValue) = 0;
			
			virtual void
			emitDestructorCall(IREmitter& irEmitter,
			                   llvm::Value* typeInfoValue,
			                   llvm::Value* objectValue) = 0;
			
		};
		
	}
	
}

#endif
