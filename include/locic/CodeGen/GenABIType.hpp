#ifndef LOCIC_CODEGEN_GENABITYPE_HPP
#define LOCIC_CODEGEN_GENABITYPE_HPP

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Type.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm_abi::Type templateGeneratorABIType();
		
		llvm_abi::Type typeInfoABIType();
		
		llvm_abi::Type genABIType(Module& module, SEM::Type* type);
		
		llvm_abi::FunctionType genABIFunctionType(Module& module, SEM::Type* functionType, llvm::Type* contextPointerType);
		
	}
	
}

#endif
