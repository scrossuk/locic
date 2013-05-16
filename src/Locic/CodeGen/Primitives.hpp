#ifndef LOCIC_CODEGEN_PRIMITIVES_HPP
#define LOCIC_CODEGEN_PRIMITIVES_HPP

#include <llvm/Function.h>
#include <llvm/Type.h>

#include <Locic/SEM.hpp>
#include <Locic/CodeGen/Module.hpp>
#include <Locic/CodeGen/TargetInfo.hpp>

namespace Locic {

	namespace CodeGen {
	
		void createPrimitiveSizeOf(Module& module, const std::string& name, llvm::Function& llvmFunction);
		
		void createPrimitiveMethod(Module& module, const std::string& typeName,
								   const std::string& methodName, llvm::Function& llvmFunction);
								   
		llvm::Type* getPrimitiveType(const Module& module, const std::string& name);
		
	}
	
}

#endif
