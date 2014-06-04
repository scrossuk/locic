#ifndef LOCIC_CODEGEN_GENTYPE_HPP
#define LOCIC_CODEGEN_GENTYPE_HPP

#include <locic/SEM.hpp>

#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Type* genArgType(Module& module, SEM::Type* type);
		
		llvm::FunctionType* genFunctionType(Module& module, SEM::Type* type);
		
		llvm::Type* genPointerType(Module& module, SEM::Type* targetType);
		
		llvm::Type* genType(Module& module, SEM::Type* type);
		
		llvm::DIType genDebugType(Module& module, SEM::Type* type);
		
	}
	
}

#endif
