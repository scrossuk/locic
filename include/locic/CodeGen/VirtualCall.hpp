#ifndef LOCIC_CODEGEN_VIRTUALCALL_HPP
#define LOCIC_CODEGEN_VIRTUALCALL_HPP

#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		namespace VirtualCall {
		
			llvm::Constant* generateVTableSlot(Module& module, SEM::Type* parentType, const std::vector<SEM::Function*>& methods);
			
			llvm::Value* generateCall(Function& function, llvm::Value* objectPtr, llvm::Value* vtablePtr, llvm::Value* hashValue, const std::vector<llvm::Value*>& args);
		
		}
	}
	
}

#endif
