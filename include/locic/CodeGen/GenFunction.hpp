#ifndef LOCIC_CODEGEN_GENFUNCTION_HPP
#define LOCIC_CODEGEN_GENFUNCTION_HPP

namespace locic {
	
	class String;
	
	namespace SEM {
		
		class Function;
		class FunctionType;
		class TemplateVar;
		class TypeInstance;
		class Type;
		
	}
	
	namespace CodeGen {
		
		class Module;
		
		llvm::GlobalValue::LinkageTypes getFunctionLinkage(const SEM::TypeInstance* typeInstance, const SEM::Function* function);
		
		llvm::GlobalValue::LinkageTypes getTypeInstanceLinkage(const SEM::TypeInstance* typeInstance);
		
		llvm::Function* genFunctionDecl(Module& module, const SEM::TypeInstance* typeInstance, SEM::Function* function);
		
		llvm::Function* genUserFunctionDecl(Module& module, const SEM::TypeInstance* typeInstance, SEM::Function* function);
		
		llvm::Function* genFunctionDef(Module& module, const SEM::TypeInstance* typeInstance, SEM::Function* function);
		
		llvm::Function* genTemplateFunctionStub(Module& module, const SEM::TemplateVar* templateVar, const String& functionName,
			SEM::FunctionType functionType, llvm::DebugLoc debugLoc);
		
	}
	
}

#endif
