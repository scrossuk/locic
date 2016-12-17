#ifndef LOCIC_CODEGEN_SEMCODEEMITTER_HPP
#define LOCIC_CODEGEN_SEMCODEEMITTER_HPP

#include <locic/CodeGen/PendingResult.hpp>

namespace locic {
	
	class MethodID;
	
	namespace AST {
		
		class Function;
		
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
			                      const AST::Function& function,
			                      bool isInnerMethod);
			
			llvm::Value*
			emitBuiltInFunctionContents(MethodID methodID,
			                            bool isInnerMethod,
			                            const SEM::TypeInstance* typeInstance,
			                            const AST::Function& function,
			                            PendingResultArray args,
			                            llvm::Value* hintResultValue);
			
			void emitBuiltInFunctionCode(const SEM::TypeInstance* typeInstance,
			                             const AST::Function& function,
			                             bool isInnerMethod);
			
			void emitUserFunctionCode(const AST::Function& function);
			
			void emitParameterAllocas(const AST::Function& function);
			
			void emitScopeFunctionCode(const AST::Function& function);
			
		private:
			Function& functionGenerator_;
			
		};
		
	}
	
}

#endif
