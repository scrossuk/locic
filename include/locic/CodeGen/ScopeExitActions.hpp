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
		
		llvm::ConstantInt* getUnwindStateValue(Module& module, UnwindState state);
		
		bool isActiveAction(const UnwindAction& unwindAction, bool isExceptionState, bool isRethrow);
		
		bool isTerminatorAction(const UnwindAction& unwindAction, UnwindState unwindState);
		
		void performScopeExitAction(Function& function, size_t position, bool isExceptionState, bool isRethrow);
		
		void genAllScopeExitActions(Function& function, bool isExceptionState = false, bool isRethrow = false);
		
		llvm::BasicBlock* genUnwind(Function& function, UnwindState unwindState);
		
		class ScopeLifetime {
			public:
				ScopeLifetime(Function& function);
				~ScopeLifetime();
				
			private:
				Function& function_;
				
		};
		
		class StatementLifetime {
			public:
				StatementLifetime(Function& function);
				~StatementLifetime();
				
			private:
				Function& function_;
				
		};
		
	}
	
}

#endif
