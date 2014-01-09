#ifndef LOCIC_CODEGEN_MEMORY_HPP
#define LOCIC_CODEGEN_MEMORY_HPP

#include <locic/SEM.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		/**
		 * \brief Zero a stack object.
		 * 
		 * This will zero out the memory pointed to by the
		 * value for the type given.
		 */
		void genZero(Function& function, SEM::Type* type, llvm::Value* value);
		
		/**
		 * \brief Create a stack object.
		 * 
		 * This will allocate stack space for the given
		 * type, and return a pointer to that space. The
		 * memory space will be zeroed, to ensure defined
		 * behaviour with the store operation (see 'genStore').
		 */
		llvm::Value* genAlloca(Function& function, SEM::Type* type);
		
		/**
		 * \brief Load a value from a memory location.
		 * 
		 * For most primitive types, this function will
		 * generated a load instruction. However, otherwise
		 * this function typically returns the pointer passed
		 * to it as-is, since class types should always be
		 * handled as pointers.
		 */
		llvm::Value* genLoad(Function& function, llvm::Value* source, SEM::Type* type);
		
		/**
		 * \brief Store a value into a memory location.
		 * 
		 * As with the load function, this handles both
		 * value types (such as primitives) by generating
		 * a normal store, but also handles reference types
		 * (such as classes) by copying the memory from
		 * one pointer to another.
		 */
		void genStore(Function& function, llvm::Value* value, llvm::Value* var, SEM::Type* unresolvedType);
		
		/**
		 * \brief Store a value into a variable.
		 * 
		 * Stores a value into the given memory location,
		 * while also performing any necessary conversion
		 * operations needed to create implicit lval types.
		 */
		void genStoreVar(Function& function, llvm::Value* value, llvm::Value* var, SEM::Var* semVar);
		
	}
	
}

#endif
