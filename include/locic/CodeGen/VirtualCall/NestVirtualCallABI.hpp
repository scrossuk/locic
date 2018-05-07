#ifndef LOCIC_CODEGEN_VIRTUALCALL_NESTVIRTUALCALLABI_HPP
#define LOCIC_CODEGEN_VIRTUALCALL_NESTVIRTUALCALLABI_HPP

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
		 * \brief Nest Virtual Call ABI
		 */
		class NestVirtualCallABI: public VirtualCallABI {
		public:
			NestVirtualCallABI(Module& module);
			~NestVirtualCallABI();
			
			ArgInfo
			getStubArgInfo();
			
			llvm::AttributeList
			conflictResolutionStubAttributes(const llvm::AttributeList& existingAttributes);
			
			llvm::Constant*
			emitVTableSlot(const AST::TypeInstance& typeInstance,
			               llvm::ArrayRef<AST::Function*> methods);
		
			llvm::Value*
			emitRawCall(IREmitter& irEmitter,
			            const ArgInfo& argInfo,
			            VirtualMethodComponents methodComponents,
			            llvm::ArrayRef<llvm::Value*> args,
			            llvm::Value* returnVarPointer);
			
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
