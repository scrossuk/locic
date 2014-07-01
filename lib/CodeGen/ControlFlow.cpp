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
			const auto& unwindStack = function.unwindStack();
			llvm::BasicBlock* breakBlock = nullptr;
			
			// Call all destructors until the next control flow point.
			for (size_t i = 0; i < unwindStack.size(); i++) {
				const size_t pos = unwindStack.size() - i - 1;
				const auto& action = unwindStack.at(pos);
				
				if (action.isControlFlow()) {
					breakBlock = action.breakBlock();
					break;
				}
				
				const bool isExceptionState = false;
				const bool isRethrow = false;
				performScopeExitAction(function, pos, isExceptionState, isRethrow);
			}
			
			assert(breakBlock != nullptr);
			
			function.getBuilder().CreateBr(breakBlock);
		}
		
		void genControlFlowContinue(Function& function) {
			const auto& unwindStack = function.unwindStack();
			llvm::BasicBlock* continueBlock = nullptr;
			
			// Perform all exit actions until the next control flow point.
			for (size_t i = 0; i < unwindStack.size(); i++) {
				const size_t pos = unwindStack.size() - i - 1;
				const auto& action = unwindStack.at(pos);
				
				if (action.isControlFlow()) {
					continueBlock = action.continueBlock();
					break;
				}
				
				const bool isExceptionState = false;
				const bool isRethrow = false;
				performScopeExitAction(function, pos, isExceptionState, isRethrow);
			}
			
			assert(continueBlock != nullptr);
			
			function.getBuilder().CreateBr(continueBlock);
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

