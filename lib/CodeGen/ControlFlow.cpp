#include <vector>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/ControlFlow.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		void genControlFlowBreak(Function& function) {
			genUnwind(function, UnwindStateBreak);
		}
		
		void genControlFlowContinue(Function& function) {
			genUnwind(function, UnwindStateContinue);
		}
		
		ControlFlowScope::ControlFlowScope(Function& function, llvm::BasicBlock* breakBlock, llvm::BasicBlock* continueBlock)
			: function_(function) {
			assert(breakBlock != nullptr && continueBlock != nullptr);
			function_.pushUnwindAction(UnwindAction::ControlFlow(breakBlock, continueBlock));
		}
		
		ControlFlowScope::~ControlFlowScope() {
			assert(function_.unwindStack().back().isControlFlow());
			function_.popUnwindAction();
		}
		
	}
	
}

