#ifndef LOCIC_CODEGEN_GENTYPE_HPP
#define LOCIC_CODEGEN_GENTYPE_HPP

namespace locic {
	
	namespace SEM {
		
		class FunctionType;
		class Type;
		
	}
	
	namespace CodeGen {
		
		class Module;
		
		llvm::Type* genArgType(Module& module, const SEM::Type* type);
		
		llvm::FunctionType* genFunctionType(Module& module, SEM::FunctionType type);
		
		llvm::PointerType* genPointerType(Module& module, const SEM::Type* targetType);
		
		llvm::Type* genType(Module& module, const SEM::Type* type);
		
		llvm::DIType genDebugFunctionType(Module& module, SEM::FunctionType type);
		
		llvm::DIType genDebugType(Module& module, const SEM::Type* type);
		
	}
	
}

#endif
