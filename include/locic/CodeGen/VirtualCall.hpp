#ifndef LOCIC_CODEGEN_VIRTUALCALL_HPP
#define LOCIC_CODEGEN_VIRTUALCALL_HPP

#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		namespace VirtualCall {
		
			llvm::Value* generateCall(Function& function, SEM::Value* methodValue, const std::vector<SEM::Value*>& args);
		
			llvm::Constant* generateVTableSlot(Module& module, SEM::Type* parentType, const std::vector<SEM::Function*>& methods);
		
		}
	}
	
}

#endif
