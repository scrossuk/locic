#ifndef LOCIC_CODEGEN_DESTRUCTOR_HPP
#define LOCIC_CODEGEN_DESTRUCTOR_HPP

#include <locic/SEM.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		bool typeHasDestructor(Module& module, SEM::TypeInstance* typeInstance);
		
		void genDestructorCall(Function& function, SEM::Type* type, llvm::Value* value);
		
		llvm::FunctionType* destructorFunctionType(Module& module, SEM::TypeInstance* typeInstance);
		
		llvm::Function* genDestructorFunction(Module& module, SEM::TypeInstance* typeInstance);
		
	}
	
}

#endif
