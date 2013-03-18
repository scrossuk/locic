#ifndef LOCIC_CODEGEN_PRIMITIVES_HPP
#define LOCIC_CODEGEN_PRIMITIVES_HPP

#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Type.h>

#include <Locic/SEM.hpp>
#include <Locic/CodeGen/TargetInfo.hpp>

namespace Locic{
	
	namespace CodeGen{
		
		void createPrimitiveSizeOf(llvm::Module& module, const std::string& name, llvm::Function* function);
		
		void createPrimitiveMethod(llvm::Module& module, const std::string& typeName,
			const std::string& methodName, llvm::Function* function);
		
		llvm::Type* createPrimitiveType(llvm::Module& module, SEM::TypeInstance* type);
		
	}
	
}

#endif
