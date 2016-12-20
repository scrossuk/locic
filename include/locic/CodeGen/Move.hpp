#ifndef LOCIC_CODEGEN_MOVE_HPP
#define LOCIC_CODEGEN_MOVE_HPP

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace AST {
		
		class Type;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		/**
		 * \brief Make an i8* to a move destination.
		 * 
		 * This just adds the position value to the destination start value
		 * (which is a pointer to the beginning of the top-most object being moved).
		 */
		llvm::Value* makeMoveDest(Function& function, llvm::Value* startDestValue, llvm::Value* positionValue);
		
		ArgInfo moveArgInfo(Module& module, const AST::TypeInstance* typeInstance);
		
		void genBasicMove(Function& function,
		                  const AST::Type* type,
		                  llvm::Value* sourceValue,
		                  llvm::Value* startDestValue,
		                  llvm::Value* positionValue);
		
		void genMoveCall(Function& function, const AST::Type* type, llvm::Value* sourceValue, llvm::Value* destValue, llvm::Value* positionValue);
		
		void genUnionMove(Function& function, const AST::TypeInstance* typeInstance);
		
		void genCallUserMoveFunction(Function& functionGenerator,
		                             const AST::TypeInstance& typeInstance,
		                             llvm::Value* const sourceValue,
		                             llvm::Value* const destValue,
		                             llvm::Value* const positionValue);
		
		llvm::Function* genVTableMoveFunction(Module& module, const AST::TypeInstance* typeInstance);
		
		llvm::Function* genMoveFunctionDecl(Module& module, const AST::TypeInstance* typeInstance);
		
		llvm::Function* genMoveFunctionDef(Module& module, const AST::TypeInstance* typeInstance);
		
	}
	
}

#endif
