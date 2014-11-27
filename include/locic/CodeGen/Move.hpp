#ifndef LOCIC_CODEGEN_MOVE_HPP
#define LOCIC_CODEGEN_MOVE_HPP

#include <locic/SEM.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		/**
		 * \brief Query whether a type has a custom move method.
		 * 
		 * This will determine whether the type has a custom
		 * move method. For class declarations this always returns
		 * true, since we must assume they have a custom move
		 * method. For known types they will have a custom move
		 * method if one of their child types has a custom move
		 * method, so this will also return true in that case.
		 */
		bool typeHasCustomMove(Module& module, const SEM::Type* type);
		
		/**
		 * \brief Query whether a type instance has a custom move method.
		 * 
		 * This will determine whether the type has a custom
		 * move method. For class declarations this always returns
		 * true, since we must assume they have a custom move
		 * method. For known types they will have a custom move
		 * method if one of their child types has a custom move
		 * method, so this will also return true in that case.
		 */
		bool typeInstanceHasCustomMove(Module& module, SEM::TypeInstance* typeInstance);
		
		/**
		 * \brief Make an i8* to a move destination.
		 * 
		 * This just adds the position value to the destination start value
		 * (which is a pointer to the beginning of the top-most object being moved).
		 */
		llvm::Value* makeRawMoveDest(Function& function, llvm::Value* startDestValue, llvm::Value* positionValue);
		
		/**
		 * \brief Make an T* to a move destination.
		 * 
		 * This is a convenience method that casts the result of makeRawMoveDest
		 * to a pointer to a generated SEM type.
		 */
		llvm::Value* makeMoveDest(Function& function, llvm::Value* startDestValue, llvm::Value* positionValue, const SEM::Type* type);
		
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
		
		ArgInfo moveArgInfo(Module& module, SEM::TypeInstance* typeInstance);
		
		void genMoveCall(Function& function, const SEM::Type* type, llvm::Value* sourceValue, llvm::Value* destValue, llvm::Value* positionValue);
		
		void genUnionMove(Function& function, SEM::TypeInstance* typeInstance);
		
		llvm::Function* genVTableMoveFunction(Module& module, SEM::TypeInstance* typeInstance);
		
		llvm::Function* genMoveFunctionDecl(Module& module, SEM::TypeInstance* typeInstance);
		
		llvm::Function* genMoveFunctionDef(Module& module, SEM::TypeInstance* typeInstance);
		
	}
	
}

#endif
