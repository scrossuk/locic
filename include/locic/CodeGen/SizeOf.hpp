#ifndef LOCIC_CODEGEN_SIZEOF_HPP
#define LOCIC_CODEGEN_SIZEOF_HPP

#include <locic/SEM.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Function* genAlignOfFunction(Module& module, SEM::TypeInstance* typeInstance);
		
		llvm::Value* genAlignOf(Function& function, SEM::Type* type);
		
		llvm::Value* genAlignMask(Function& function, SEM::Type* type);
		
		llvm::Function* genSizeOfFunction(Module& module, SEM::TypeInstance* typeInstance);
		
		llvm::Value* genSizeOf(Function& function, SEM::Type* type);
		
		llvm::Value* makeAligned(Function& function, llvm::Value* size, llvm::Value* alignMask);
		
		llvm::Value* genMemberOffset(Function& function, SEM::Type* type, size_t memberIndex);
		
	}
	
}

#endif
