#ifndef LOCIC_CODEGEN_VIRTUALCALL_GENERICVIRTUALCALLABI_HPP
#define LOCIC_CODEGEN_VIRTUALCALL_GENERICVIRTUALCALLABI_HPP

#include <locic/CodeGen/VirtualCallABI.hpp>

namespace locic {
	
	namespace AST {
		
		class Function;
		class FunctionType;
		class Type;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class ArgInfo;
		class IREmitter;
		class Module;
		struct VirtualMethodComponents;
		
		/**
		 * \brief Generic Virtual Call ABI
		 * 
		 * This is a 'generic' virtual call mechanism which passes the
		 * call arguments on the stack. It is therefore less efficient
		 * but easy to implement.
		 */
		class GenericVirtualCallABI: public VirtualCallABI {
		public:
			GenericVirtualCallABI(Module& module);
			~GenericVirtualCallABI();
			
			ArgInfo
			getStubArgInfo();
			
			llvm::AttributeSet
			conflictResolutionStubAttributes(const llvm::AttributeSet& existingAttributes);
			
			llvm::Value*
			makeArgsStruct(IREmitter& irEmitter,
			               llvm::ArrayRef<const AST::Type*> argTypes,
			               llvm::ArrayRef<llvm::Value*> args);
			
			llvm::Constant*
			emitVTableSlot(const AST::TypeInstance& typeInstance,
			               llvm::ArrayRef<AST::Function*> methods);
		
			void
			emitCallWithReturnVar(IREmitter& irEmitter,
			                      const AST::FunctionType functionType,
			                      llvm::Value* returnVarPointer,
			                      VirtualMethodComponents methodComponents,
			                      llvm::ArrayRef<llvm::Value*> args);
			
			llvm::Value*
			emitCall(IREmitter& irEmitter,
			         AST::FunctionType functionType,
			         VirtualMethodComponents methodComponents,
			         llvm::ArrayRef<llvm::Value*> args,
			         llvm::Value* resultPtr);
			
			llvm::Value*
			emitCountFnCall(IREmitter& irEmitter,
			                llvm::Value* typeInfoValue,
			                CountFnKind kind);
			
		private:
			Module& module_;
			
		};
		
	}
	
}

#endif
