#ifndef LOCIC_CODEGEN_VIRTUALCALL_HPP
#define LOCIC_CODEGEN_VIRTUALCALL_HPP

#include <locic/CodeGen/MethodInfo.hpp>

namespace locic {
	
	namespace SEM {
		
		class Type;
		
	}
	
	namespace CodeGen {
		
		class Module;
		
		namespace VirtualCall {
			
			llvm::Constant* generateVTableSlot(Module& module, const SEM::TypeInstance* typeInstance, llvm::ArrayRef<SEM::Function*> methods);
			
			llvm::Value* generateCall(Function& function, const SEM::Type* functionType, VirtualMethodComponents methodComponents,
				llvm::ArrayRef<llvm::Value*> args, Optional<llvm::DebugLoc> debugLoc = None, llvm::Value* hintResultValue = nullptr);
			
			enum CountFnKind {
				ALIGNOF,
				SIZEOF
			};
			
			llvm::Value* generateCountFnCall(Function& function, llvm::Value* typeInfoValue, CountFnKind kind, Optional<llvm::DebugLoc> debugLoc = None);
			
			void generateMoveCall(Function& function, llvm::Value* typeInfoValue, llvm::Value* sourceValue,
				llvm::Value* destValue, llvm::Value* positionValue, Optional<llvm::DebugLoc> debugLoc = None);
			
			void generateDestructorCall(Function& function, llvm::Value* typeInfoValue, llvm::Value* objectValue, Optional<llvm::DebugLoc> debugLoc = None);
			
		}
	}
	
}

#endif
