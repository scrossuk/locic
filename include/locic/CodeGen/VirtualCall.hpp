#ifndef LOCIC_CODEGEN_VIRTUALCALL_HPP
#define LOCIC_CODEGEN_VIRTUALCALL_HPP

#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		namespace VirtualCall {
			
			llvm::Constant* generateVTableSlot(Module& module, SEM::TypeInstance* typeInstance, const std::vector<SEM::Function*>& methods);
			
			llvm::Value* generateCall(Function& function, SEM::Type* functionType, llvm::Value* interfaceMethodValue, const std::vector<llvm::Value*>& args);
			
			llvm::Value* generateTypeInfoCall(Function& function, llvm::Type* returnType, llvm::Value* typeInfo, const std::string& name, const std::vector<llvm::Value*>& args);
			
			enum CountFnKind {
				ALIGNOF,
				SIZEOF
			};
			
			llvm::Value* generateCountFnCall(Function& function, llvm::Value* typeInfoValue, CountFnKind kind);
			
		}
	}
	
}

#endif
