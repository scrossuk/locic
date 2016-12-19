#ifndef LOCIC_CODEGEN_MEMORY_HPP
#define LOCIC_CODEGEN_MEMORY_HPP

namespace locic {
	
	namespace AST {
		
		class Type;
		class Var;
		
	}
	
	namespace CodeGen {
		
		class Function;
		
		/**
		 * \brief Store a value into a variable.
		 * 
		 * Stores a value into the given memory location,
		 * while also performing any necessary conversion
		 * operations needed to create implicit lval types.
		 */
		void genStoreVar(Function& function, llvm::Value* value, llvm::Value* var, AST::Var* semVar);
		
		/**
		 * \brief Allocate and store a value on the stack,
		 *        in order to get a pointer to it.
		 * 
		 * (This is typically used to generate a pointer
		 * passed as the context pointer to methods.)
		 */
		llvm::Value* genValuePtr(Function& function, llvm::Value* value, const AST::Type* type, llvm::Value* const hintResultValue = nullptr);
		
	}
	
}

#endif
