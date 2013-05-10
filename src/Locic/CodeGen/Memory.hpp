#ifndef LOCIC_CODEGEN_MEMORY_HPP
#define LOCIC_CODEGEN_MEMORY_HPP

#include <Locic/SEM.hpp>
#include <Locic/CodeGen/Function.hpp>
#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
	
		llvm::Value* genAlloca(Function& function, SEM::Type* type);
		
		llvm::Value* genStore(Function& function, llvm::Value* value, llvm::Value* var, SEM::Type* type);
		
		llvm::Value* genLoad(Function& function, llvm::Value* var, SEM::Type* type);
		
	}
	
}

#endif
