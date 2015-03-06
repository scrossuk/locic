#ifndef LOCIC_CODEGEN_GENVALUE_HPP
#define LOCIC_CODEGEN_GENVALUE_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/Function.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Value* genValue(Function& function, const SEM::Value& value, llvm::Value* hintResultValue = nullptr);
		
	}
	
}

#endif
