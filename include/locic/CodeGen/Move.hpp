#ifndef LOCIC_CODEGEN_MOVE_HPP
#define LOCIC_CODEGEN_MOVE_HPP

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace SEM {
		
		class TypeInstance;
		class Type;
		
	}
	
	namespace CodeGen {
		
		/**
		 * \brief Make an i8* to a move destination.
		 * 
		 * This just adds the position value to the destination start value
		 * (which is a pointer to the beginning of the top-most object being moved).
		 */
		llvm::Value* makeMoveDest(Function& function, llvm::Value* startDestValue, llvm::Value* positionValue);
		
		/**
		 * \brief Move a value by loading it from a memory location.
		 * 
		 * This function can be used to provide move operations; it will
		 * generate a trivial load where possible but may return the
		 * value pointer (as passed to it) if the type has a custom
		 * move method, since this means the object must be kept in memory.
		 */
		llvm::Value* genMoveLoad(Function& function, llvm::Value* var, const SEM::Type* type);
		
		/**
		 * \brief Move a value by storing it into a memory location.
		 * 
		 * This function can be used to provide move operations; it will
		 * generate a trivial store where possible but will invoke the
		 * move method if the type has a custom move method.
		 */
		void genMoveStore(Function& function, llvm::Value* value, llvm::Value* var, const SEM::Type* type);
		
		ArgInfo moveArgInfo(Module& module, const SEM::TypeInstance* typeInstance);
		
		void genBasicMove(Function& function,
		                  const SEM::Type* type,
		                  llvm::Value* sourceValue,
		                  llvm::Value* startDestValue,
		                  llvm::Value* positionValue);
		
		void genMoveCall(Function& function, const SEM::Type* type, llvm::Value* sourceValue, llvm::Value* destValue, llvm::Value* positionValue);
		
		void genUnionMove(Function& function, const SEM::TypeInstance* typeInstance);
		
		void genCallUserMoveFunction(Function& functionGenerator,
		                             const SEM::TypeInstance& typeInstance,
		                             llvm::Value* const sourceValue,
		                             llvm::Value* const destValue,
		                             llvm::Value* const positionValue);
		
		llvm::Function* genVTableMoveFunction(Module& module, const SEM::TypeInstance* typeInstance);
		
		llvm::Function* genMoveFunctionDecl(Module& module, const SEM::TypeInstance* typeInstance);
		
		llvm::Function* genMoveFunctionDef(Module& module, const SEM::TypeInstance* typeInstance);
		
	}
	
}

#endif
