#ifndef LOCIC_CODEGEN_GENTYPE_HPP
#define LOCIC_CODEGEN_GENTYPE_HPP

#include <Locic/SEM.hpp>

#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
		
		llvm::FunctionType* genFunctionType(const Module& module, SEM::Type* type, llvm::Type* contextPointerType = NULL);
		
		llvm::Type* getTypeInstancePointer(const Module& module, SEM::TypeInstance* typeInstance);
	
		llvm::Type* genType(const Module& module, SEM::Type* type);
		
	}
	
}

#endif
