#ifndef LOCIC_CODEGEN_SCOPEEXITACTIONS_HPP
#define LOCIC_CODEGEN_SCOPEEXITACTIONS_HPP

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/UnwindState.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Value* getIsCurrentUnwindState(Function& function, UnwindState state);
		
		llvm::Value* getIsCurrentExceptState(Function& function);
		
		void setCurrentUnwindState(Function& function, UnwindState state);
		
		llvm::ConstantInt* getUnwindStateValue(Module& module, UnwindState state);
		
		bool anyUnwindCleanupActions(Function& function, UnwindState unwindState);
		
		bool anyUnwindActions(Function& function, UnwindState unwindState);
		
		void performScopeExitAction(Function& function, size_t position, UnwindState unwindState);
		
		llvm::BasicBlock* genUnwindBlock(Function& function, UnwindState unwindState);
		
		void genUnwind(Function& function, UnwindState unwindState);
		
		class ScopeLifetime {
			public:
				ScopeLifetime(Function& function);
				~ScopeLifetime();
				
			private:
				Function& function_;
				llvm::BasicBlock* scopeEndBB_;
				
		};
		
	}
	
}

#endif
