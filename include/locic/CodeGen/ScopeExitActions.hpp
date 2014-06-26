#ifndef LOCIC_CODEGEN_SCOPEEXITACTIONS_HPP
#define LOCIC_CODEGEN_SCOPEEXITACTIONS_HPP

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		enum UnwindState {
			UnwindStateNormal = 0,
			UnwindStateReturn = 1,
			UnwindStateBreak = 2,
			UnwindStateContinue = 3,
			UnwindStateThrow = 4,
			UnwindStateRethrow = 5,
		};
		
		llvm::Value* getIsCurrentUnwindState(Function& function, UnwindState state);
		
		llvm::Value* getIsCurrentExceptState(Function& function);
		
		void setCurrentUnwindState(Function& function, UnwindState state);
		
		llvm::Value* getUnwindStateValue(Module& module, UnwindState state);
		
		llvm::BasicBlock* getNextNormalUnwindBlock(Function& function);
		
		llvm::BasicBlock* getNextExceptUnwindBlock(Function& function);
		
		class FunctionLifetime {
			public:
				FunctionLifetime(Function& function);
				~FunctionLifetime();
				
			private:
				Function& function_;
			
		};
		
		class ScopeLifetime {
			public:
				ScopeLifetime(Function& function);
				~ScopeLifetime();
				
			private:
				Function& function_;
				llvm::BasicBlock* scopeExitBB_;
			
		};
		
		class StatementLifetime {
			public:
				StatementLifetime(Function& function);
				~StatementLifetime();
				
			private:
				Function& function_;
				llvm::BasicBlock* statementExitBB_;
			
		};
		
	}
	
}

#endif
