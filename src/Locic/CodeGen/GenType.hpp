#ifndef LOCIC_CODEGEN_GENTYPE_HPP
#define LOCIC_CODEGEN_GENTYPE_HPP

#include <Locic/SEM.hpp>

#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
		
		bool resolvesToClassType(Module& module, SEM::Type* type);
		
		llvm::FunctionType* genFunctionType(Module& module, SEM::Type* type, llvm::Type* contextPointerType = NULL);
		
		llvm::Type* getTypeInstancePointer(Module& module, SEM::TypeInstance* typeInstance,
			const std::vector<SEM::Type*>& templateArguments);
	
		llvm::Type* genType(Module& module, SEM::Type* type);
		
	}
	
}

#endif
