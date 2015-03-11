#ifndef LOCIC_CODEGEN_GENFUNCTION_HPP
#define LOCIC_CODEGEN_GENFUNCTION_HPP

#include <locic/SEM.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {
	
	class String;
	
	namespace CodeGen {
		
		llvm::GlobalValue::LinkageTypes getFunctionLinkage(SEM::Function* function);
		
		llvm::GlobalValue::LinkageTypes getTypeInstanceLinkage(const SEM::TypeInstance* typeInstance);
		
		llvm::Function* genFunctionDecl(Module& module, const SEM::TypeInstance* typeInstance, SEM::Function* function);
		
		llvm::Function* genFunctionDef(Module& module, const SEM::TypeInstance* typeInstance, SEM::Function* function);
		
		llvm::Function* genTemplateFunctionStub(Module& module, SEM::TemplateVar* templateVar, const String& functionName, const SEM::Type* functionType, Optional<llvm::DebugLoc> debugLoc);
		
	}
	
}

#endif
