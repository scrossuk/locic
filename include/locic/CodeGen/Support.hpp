#ifndef LOCIC_CODEGEN_SUPPORT_HPP
#define LOCIC_CODEGEN_SUPPORT_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::StructType* vtableType(Module& module);
		
	}
	
}

#endif
