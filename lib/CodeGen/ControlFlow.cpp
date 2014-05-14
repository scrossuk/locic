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
				performScopeExitAction(function, action, isExceptionState);
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
				performScopeExitAction(function, action, isExceptionState);
			}
			
			assert(continueBlock != nullptr);
			
			function.getBuilder().CreateBr(continueBlock);
		}
		
		ControlFlowScope::ControlFlowScope(UnwindStack& unwindStack, llvm::BasicBlock* breakBlock, llvm::BasicBlock* continueBlock)
			: unwindStack_(unwindStack) {
				assert(breakBlock != nullptr && continueBlock != nullptr);
				unwindStack_.push_back(UnwindAction::ControlFlow(breakBlock, continueBlock));
			}
		
		ControlFlowScope::~ControlFlowScope() {
			assert(unwindStack_.back().isControlFlow());
			unwindStack_.pop_back();
		}
		
	}
	
}

