#include <vector>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/ControlFlow.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		void genControlFlowBreak(Function& function) {
			IREmitter(function).emitUnwind(UnwindStateBreak);
		}
		
		void genControlFlowContinue(Function& function) {
			IREmitter(function).emitUnwind(UnwindStateContinue);
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

