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
			// Switch the unwind state to 'break'
			// and jump to the first unwind block.
			setCurrentUnwindState(function, UnwindStateBreak);
			function.getBuilder().CreateBr(getNextUnwindBlock(function));
		}
		
		void genControlFlowContinue(Function& function) {
			// Switch the unwind state to 'continue'
			// and jump to the first unwind block.
			setCurrentUnwindState(function, UnwindStateContinue);
			function.getBuilder().CreateBr(getNextUnwindBlock(function));
		}
		
		llvm::BasicBlock* genControlFlowBlock(Function& function, llvm::BasicBlock* breakBB, llvm::BasicBlock* continueBB) {
			const auto currentBB = function.getSelectedBasicBlock();
			
			const auto unwindBB = function.createBasicBlock("controlFlow");
			const auto isBreakBB = function.createBasicBlock("");
			const auto isContinueBB = function.createBasicBlock("");
			const auto testIsContinueBB = function.createBasicBlock("");
			const auto nextUnwindBB = getNextUnwindBlock(function);
			
			function.selectBasicBlock(unwindBB);
			const auto isBreakState = getIsCurrentUnwindState(function, UnwindStateBreak);
			function.getBuilder().CreateCondBr(isBreakState, isBreakBB, testIsContinueBB);
			
			function.selectBasicBlock(testIsContinueBB);
			const auto isContinueState = getIsCurrentUnwindState(function, UnwindStateContinue);
			function.getBuilder().CreateCondBr(isContinueState, isContinueBB, nextUnwindBB);
			
			function.selectBasicBlock(isBreakBB);
			// Set unwind state to 'normal'.
			setCurrentUnwindState(function, UnwindStateNormal);
			function.getBuilder().CreateBr(breakBB);
			
			function.selectBasicBlock(isContinueBB);
			// Set unwind state to 'normal'.
			setCurrentUnwindState(function, UnwindStateNormal);
			function.getBuilder().CreateBr(continueBB);
			
			function.selectBasicBlock(currentBB);
			
			return unwindBB;
		}
		
		ControlFlowScope::ControlFlowScope(Function& function, llvm::BasicBlock* breakBB, llvm::BasicBlock* continueBB)
			: unwindStack_(function.unwindStack()) {
				assert(breakBB != nullptr && continueBB != nullptr);
				
				const auto actionBB = genControlFlowBlock(function, breakBB, continueBB);
				unwindStack_.push_back(UnwindAction::ControlFlow(actionBB));
			}
		
		ControlFlowScope::~ControlFlowScope() {
			assert(unwindStack_.back().isControlFlow());
			unwindStack_.pop_back();
		}
		
	}
	
}

