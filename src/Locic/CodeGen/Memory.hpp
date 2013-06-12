#ifndef LOCIC_CODEGEN_MEMORY_HPP
#define LOCIC_CODEGEN_MEMORY_HPP

#include <Locic/SEM.hpp>
#include <Locic/CodeGen/Function.hpp>
#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
	
		void genZeroStore(Function& function, llvm::Value* value, SEM::Type* type);
		
		llvm::Value* genAlloca(Function& function, SEM::Type* type);
		
		void genMoveStore(Function& function, llvm::Value* source, llvm::Value* dest, SEM::Type* type);
		
		llvm::Value* genLoad(Function& function, llvm::Value* source, SEM::Type* type);
		
	}
	
}

#endif
