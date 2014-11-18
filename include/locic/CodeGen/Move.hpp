#ifndef LOCIC_CODEGEN_MOVE_HPP
#define LOCIC_CODEGEN_MOVE_HPP

#include <locic/SEM.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		bool typeHasCustomMove(Module& module, const SEM::Type* type);
		
		bool typeInstanceHasCustomMove(Module& module, SEM::TypeInstance* const typeInstance);
		
		ArgInfo moveArgInfo(Module& module, SEM::TypeInstance* typeInstance);
		
		void genMoveCall(Function& function, const SEM::Type* type, llvm::Value* sourceValue, llvm::Value* destValue, llvm::Value* positionValue);
		
		void genUnionMove(Function& function, SEM::TypeInstance* typeInstance);
		
		llvm::Function* genVTableMoveFunction(Module& module, SEM::TypeInstance* typeInstance);
		
		llvm::Function* genMoveFunctionDecl(Module& module, SEM::TypeInstance* typeInstance);
		
		llvm::Function* genMoveFunctionDef(Module& module, SEM::TypeInstance* typeInstance);
		
	}
	
}

#endif
