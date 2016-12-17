#ifndef LOCIC_CODEGEN_SEMCODEEMITTER_HPP
#define LOCIC_CODEGEN_SEMCODEEMITTER_HPP

#include <locic/CodeGen/PendingResult.hpp>

namespace locic {
	
	class MethodID;
	
	namespace AST {
		
		class FunctionDecl;
		
	}
	
	namespace SEM {
		
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class Function;
		
		class SEMCodeEmitter {
		public:
			SEMCodeEmitter(Function& functionGenerator);
			
			void emitFunctionCode(const SEM::TypeInstance* typeInstance,
			                      const AST::FunctionDecl& function,
			                      bool isInnerMethod);
			
			llvm::Value*
			emitBuiltInFunctionContents(MethodID methodID,
			                            bool isInnerMethod,
			                            const SEM::TypeInstance* typeInstance,
			                            const AST::FunctionDecl& function,
			                            PendingResultArray args,
			                            llvm::Value* hintResultValue);
			
			void emitBuiltInFunctionCode(const SEM::TypeInstance* typeInstance,
			                             const AST::FunctionDecl& function,
			                             bool isInnerMethod);
			
			void emitUserFunctionCode(const AST::FunctionDecl& function);
			
			void emitParameterAllocas(const AST::FunctionDecl& function);
			
			void emitScopeFunctionCode(const AST::FunctionDecl& function);
			
		private:
			Function& functionGenerator_;
			
		};
		
	}
	
}

#endif
