#ifndef LOCIC_CODEGEN_PRIMITIVES_HPP
#define LOCIC_CODEGEN_PRIMITIVES_HPP

#include <llvm/Module.h>
#include <llvm/Type.h>

#include <Locic/SEM.hpp>
#include <Locic/CodeGen/TargetInfo.hpp>

namespace Locic{
	
	namespace CodeGen{
		
		void createPrimitiveMethods(llvm::Module& module);
		
		llvm::Type* createPrimitiveType(llvm::Module& module, SEM::TypeInstance* type);
		
	}
	
}

#endif
