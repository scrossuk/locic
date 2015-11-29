#ifndef LOCIC_CODEGEN_MEMORY_HPP
#define LOCIC_CODEGEN_MEMORY_HPP

namespace locic {
	
	namespace SEM {
		
		class Type;
		class Var;
		
	}
	
	namespace CodeGen {
		
		class Function;
		
		/**
		 * \brief Store a value into a memory location.
		 * 
		 * As with the load function, this handles both
		 * value types (such as primitives) by generating
		 * a normal store, but also handles reference types
		 * (such as classes) by copying the memory from
		 * one pointer to another.
		 */
		void genStore(Function& function, llvm::Value* value, llvm::Value* var, const SEM::Type* type);
		
		/**
		 * \brief Store a value into a variable.
		 * 
		 * Stores a value into the given memory location,
		 * while also performing any necessary conversion
		 * operations needed to create implicit lval types.
		 */
		void genStoreVar(Function& function, llvm::Value* value, llvm::Value* var, SEM::Var* semVar);
		
		/**
		 * \brief Allocate and store a value on the stack,
		 *        in order to get a pointer to it.
		 * 
		 * (This is typically used to generate a pointer
		 * passed as the context pointer to methods.)
		 */
		llvm::Value* genValuePtr(Function& function, llvm::Value* value, const SEM::Type* type, llvm::Value* const hintResultValue = nullptr);
		
	}
	
}

#endif
