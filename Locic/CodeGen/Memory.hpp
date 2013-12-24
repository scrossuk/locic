#ifndef LOCIC_CODEGEN_MEMORY_HPP
#define LOCIC_CODEGEN_MEMORY_HPP

#include <Locic/SEM.hpp>
#include <Locic/CodeGen/Function.hpp>
#include <Locic/CodeGen/Module.hpp>

namespace Locic {

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
		 * \brief Load a value.
		 * 
		 * For most primitive types, this function will
		 * generated a load instruction. However, otherwise
		 * this function typically returns the pointer passed
		 * to it as-is, since class types should always be
		 * handled as pointers.
		 */
		llvm::Value* genLoad(Function& function, llvm::Value* source, SEM::Type* type);
		
		/**
		 * \brief Store a value into a variable.
		 * 
		 * This will automatically handle types that need the
		 * existing destination value to be destroyed, and for
		 * the source value to be zeroed out (to prevent
		 * multiple destruction of the same value).
		 */
		void genStore(Function& function, llvm::Value* value, llvm::Value* var, SEM::Type* unresolvedType, bool destroyExisting = true);
		
	}
	
}

#endif
