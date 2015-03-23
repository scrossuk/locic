#ifndef LOCIC_CODEGEN_GENVAR_HPP
#define LOCIC_CODEGEN_GENVAR_HPP

namespace locic {
	
	namespace SEM {
		
		class Var;
		
	}
	
	namespace CodeGen {
		
		class Function;
		
		void genVarAlloca(Function& function, SEM::Var* var);
		
		void genVarInitialise(Function& function, SEM::Var* var, llvm::Value* initialiseValue);
		
	}
	
}

#endif
