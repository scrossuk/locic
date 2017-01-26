#ifndef LOCIC_CODEGEN_GENTYPE_HPP
#define LOCIC_CODEGEN_GENTYPE_HPP

namespace locic {
	
	namespace AST {
		
		class FunctionType;
		class Type;
		
	}
	
	namespace CodeGen {
		
		class Module;
		
		DISubroutineType genDebugFunctionType(Module& module, AST::FunctionType type);
		
		DIType genDebugType(Module& module, const AST::Type* type);
		
	}
	
}

#endif
