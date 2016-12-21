#ifndef LOCIC_CODEGEN_DESTRUCTOR_HPP
#define LOCIC_CODEGEN_DESTRUCTOR_HPP


#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		ArgInfo destructorArgInfo(Module& module, const AST::TypeInstance& typeInstance);
		
		void scheduleDestructorCall(Function& function, const AST::Type* type, llvm::Value* value);
		
		llvm::FunctionType* destructorFunctionType(Module& module, const AST::TypeInstance& typeInstance);
		
		Debug::SourcePosition getDebugDestructorPosition(Module& module, const AST::TypeInstance& typeInstance);
		
		DISubprogram genDebugDestructorFunction(Module& module, const AST::TypeInstance& typeInstance, llvm::Function* const function);
		
		llvm::Function* genDestructorFunctionDecl(Module& module, const AST::TypeInstance& typeInstance);
		
		llvm::Function* genVTableDestructorFunction(Module& module, const AST::TypeInstance& typeInstance);
		
	}
	
}

#endif
