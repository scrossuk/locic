#ifndef LOCIC_CODEGEN_GENFUNCTION_HPP
#define LOCIC_CODEGEN_GENFUNCTION_HPP

#include <string>

#include <locic/SEM.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::GlobalValue::LinkageTypes getFunctionLinkage(SEM::Function* function);
		
		llvm::GlobalValue::LinkageTypes getTypeInstanceLinkage(SEM::TypeInstance* typeInstance);
		
		llvm::Function* genFunctionDecl(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* function);
		
		llvm::Function* genFunctionDef(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* function);
		
		llvm::Function* genTemplateFunctionStub(Module& module, SEM::TemplateVar* templateVar, const std::string& functionName, const SEM::Type* functionType);
		
	}
	
}

#endif
