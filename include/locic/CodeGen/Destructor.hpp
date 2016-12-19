#ifndef LOCIC_CODEGEN_DESTRUCTOR_HPP
#define LOCIC_CODEGEN_DESTRUCTOR_HPP

#include <locic/SEM.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		ArgInfo destructorArgInfo(Module& module, const SEM::TypeInstance& typeInstance);
		
		void scheduleDestructorCall(Function& function, const AST::Type* type, llvm::Value* value);
		
		llvm::FunctionType* destructorFunctionType(Module& module, const SEM::TypeInstance& typeInstance);
		
		Debug::SourcePosition getDebugDestructorPosition(Module& module, const SEM::TypeInstance& typeInstance);
		
		DISubprogram genDebugDestructorFunction(Module& module, const SEM::TypeInstance& typeInstance, llvm::Function* const function);
		
		llvm::Function* genDestructorFunctionDecl(Module& module, const SEM::TypeInstance& typeInstance);
		
		llvm::Function* genVTableDestructorFunction(Module& module, const SEM::TypeInstance& typeInstance);
		
	}
	
}

#endif
