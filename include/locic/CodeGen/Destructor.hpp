#ifndef LOCIC_CODEGEN_DESTRUCTOR_HPP
#define LOCIC_CODEGEN_DESTRUCTOR_HPP

#include <locic/SEM.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		bool typeHasDestructor(Module& module, const SEM::Type* type);
		
		bool typeInstanceHasDestructor(Module& module, SEM::TypeInstance* typeInstance);
		
		ArgInfo destructorArgInfo(Module& module, SEM::TypeInstance* typeInstance);
		
		void genDestructorCall(Function& function, const SEM::Type* type, llvm::Value* value);
		
		void scheduleDestructorCall(Function& function, const SEM::Type* type, llvm::Value* value);
		
		llvm::FunctionType* destructorFunctionType(Module& module, SEM::TypeInstance* typeInstance);
		
		llvm::Function* genDestructorFunctionDecl(Module& module, SEM::TypeInstance* typeInstance);
		
		llvm::Function* genDestructorFunctionDef(Module& module, SEM::TypeInstance* typeInstance);
		
		llvm::Function* genVTableDestructorFunction(Module& module, SEM::TypeInstance* typeInstance);
		
	}
	
}

#endif
