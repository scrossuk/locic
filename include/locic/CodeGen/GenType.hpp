#ifndef LOCIC_CODEGEN_GENTYPE_HPP
#define LOCIC_CODEGEN_GENTYPE_HPP

namespace locic {
	
	namespace AST {
		
		class FunctionType;
		
	}
	
	namespace SEM {
		
		class Type;
		
	}
	
	namespace CodeGen {
		
		class Module;
		
		llvm::Type* genArgType(Module& module, const SEM::Type* type);
		
		llvm::FunctionType* genFunctionType(Module& module, AST::FunctionType type);
		
		llvm::Type* genType(Module& module, const SEM::Type* type);
		
		DISubroutineType genDebugFunctionType(Module& module, AST::FunctionType type);
		
		DIType genDebugType(Module& module, const SEM::Type* type);
		
	}
	
}

#endif
