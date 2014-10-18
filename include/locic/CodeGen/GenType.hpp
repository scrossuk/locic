#ifndef LOCIC_CODEGEN_GENTYPE_HPP
#define LOCIC_CODEGEN_GENTYPE_HPP

#include <locic/SEM.hpp>

#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Type* genArgType(Module& module, const SEM::Type* type);
		
		llvm::FunctionType* genFunctionType(Module& module, const SEM::Type* type);
		
		llvm::PointerType* genPointerType(Module& module, const SEM::Type* targetType);
		
		llvm::Type* genType(Module& module, const SEM::Type* type);
		
		llvm::DIType genDebugType(Module& module, const SEM::Type* type);
		
	}
	
}

#endif
