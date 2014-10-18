#ifndef LOCIC_CODEGEN_GENABITYPE_HPP
#define LOCIC_CODEGEN_GENABITYPE_HPP

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Type.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm_abi::Type* genABIArgType(Module& module, const SEM::Type* type);
		
		llvm_abi::Type* genABIType(Module& module, const SEM::Type* type);
		
	}
	
}

#endif
