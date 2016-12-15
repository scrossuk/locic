#ifndef LOCIC_CODEGEN_GENVAR_HPP
#define LOCIC_CODEGEN_GENVAR_HPP

namespace locic {
	
	namespace AST {
		
		class Var;
		
	}
	
	namespace CodeGen {
		
		class Function;
		
		void genVarAlloca(Function& function, AST::Var* var,
			llvm::Value* hintResultValue = nullptr);
		
		void genVarInitialise(Function& function, AST::Var* var, llvm::Value* initialiseValue);
		
	}
	
}

#endif
