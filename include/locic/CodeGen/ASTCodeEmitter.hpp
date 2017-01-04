#ifndef LOCIC_CODEGEN_ASTCODEEMITTER_HPP
#define LOCIC_CODEGEN_ASTCODEEMITTER_HPP

#include <locic/CodeGen/PendingResult.hpp>

namespace locic {
	
	class MethodID;
	
	namespace AST {
		
		class Function;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class Function;
		
		class ASTCodeEmitter {
		public:
			ASTCodeEmitter(Function& functionGenerator);
			
			void emitFunctionCode(const AST::TypeInstance* typeInstance,
			                      const AST::Function& function,
			                      bool isInnerMethod);
			
			llvm::Value*
			emitBuiltInFunctionContents(MethodID methodID,
			                            bool isInnerMethod,
			                            const AST::TypeInstance* typeInstance,
			                            const AST::Function& function,
			                            PendingResultArray args,
			                            llvm::Value* hintResultValue);
			
			void emitBuiltInFunctionCode(const AST::TypeInstance* typeInstance,
			                             const AST::Function& function,
			                             bool isInnerMethod);
			
			void emitUserFunctionCode(const AST::Function& function);
			
			void emitParameterAssignments(const AST::Function& function);
			
			void emitScopeFunctionCode(const AST::Function& function);
			
		private:
			Function& functionGenerator_;
			
		};
		
	}
	
}

#endif
