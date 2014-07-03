#include <stdexcept>
#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/SEM.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Exception.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenStatement.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Value* getIsCurrentUnwindState(Function& function, UnwindState state) {
			const auto currentUnwindStateValue = function.getBuilder().CreateLoad(function.unwindState());
			const auto targetUnwindStateValue = getUnwindStateValue(function.module(), state);
			return function.getBuilder().CreateICmpEQ(currentUnwindStateValue, targetUnwindStateValue);
		}
		
		llvm::Value* getIsCurrentExceptState(Function& function) {
			const auto isThrowState = getIsCurrentUnwindState(function, UnwindStateThrow);
			const auto isRethrowState = getIsCurrentUnwindState(function, UnwindStateRethrow);
			return function.getBuilder().CreateOr(isThrowState, isRethrowState);
		}
		
		void setCurrentUnwindState(Function& function, UnwindState state) {
			const auto stateValue = getUnwindStateValue(function.module(), state);
			function.getBuilder().CreateStore(stateValue, function.unwindState());
		}
		
		llvm::ConstantInt* getUnwindStateValue(Module& module, UnwindState state) {
			return ConstantGenerator(module).getI8(state);
		}
		
		namespace {
		
			void popScope(Function& function) {
				const auto& unwindStack = function.unwindStack();
				while (!unwindStack.empty()) {
					const bool isMarker = unwindStack.back().isScopeMarker();
					function.popUnwindAction();
					
					if (isMarker) {
						return;
					}
				}
				
				llvm_unreachable("Scope marker not found.");
			}
			
			bool lastInstructionTerminates(Function& function) {
				if (!function.getBuilder().GetInsertBlock()->empty()) {
					auto iterator = function.getBuilder().GetInsertPoint();
					--iterator;
					return iterator->isTerminator();
				} else {
					return false;
				}
			}
			
		}
		
		void performScopeExitAction(Function& function, size_t position, UnwindState unwindState) {
			const auto& unwindAction = function.unwindStack().at(position);
			
			if (!unwindAction.isActiveForState(unwindState)) {
				return;
			}
			
			switch (unwindAction.kind()) {
				case UnwindAction::DESTRUCTOR: {
					genDestructorCall(function, unwindAction.destructorType(), unwindAction.destructorValue());
					return;
				}
				case UnwindAction::CATCH: {
					function.getBuilder().CreateBr(unwindAction.catchBlock());
					return;
				}
				case UnwindAction::SCOPEMARKER: {
					// Just a placeholder.
					return;
				}
				case UnwindAction::FUNCTIONMARKER: {
					// Just a placeholder.
					switch (unwindState) {
						case UnwindStateThrow: {
							const auto exceptionInfo = function.getBuilder().CreateLoad(function.exceptionInfo());
							function.getBuilder().CreateResume(exceptionInfo);
							break;
						}
						case UnwindStateReturn: {
							const auto returnType = function.getLLVMFunction().getFunctionType()->getReturnType();
							if (function.getArgInfo().hasReturnVarArgument() || returnType->isVoidTy()) {
								function.getBuilder().CreateRetVoid();
							} else {
								function.getBuilder().CreateRet(function.getBuilder().CreateLoad(function.getRawReturnValue()));
							}
							break;
						}
						default:
							llvm_unreachable("Invalid unwind state for function marker action.");
					}
					return;
				}
				case UnwindAction::CONTROLFLOW: {
					switch (unwindState) {
						case UnwindStateBreak:
							function.getBuilder().CreateBr(unwindAction.breakBlock());
							break;
						case UnwindStateContinue:
							function.getBuilder().CreateBr(unwindAction.continueBlock());
							break;
						default:
							llvm_unreachable("Invalid unwind state for control flow action.");
					}
					return;
				}
				case UnwindAction::SCOPEEXIT: {
					function.pushUnwindStack(position);
					genScope(function, *(unwindAction.scopeExitScope()));
					function.popUnwindStack();
					
					if (lastInstructionTerminates(function)) {
						function.selectBasicBlock(function.createBasicBlock(""));
					}
					return;
				}
				case UnwindAction::DESTROYEXCEPTION: {
					if (unwindState == UnwindStateThrow) {
						llvm::Value* const values[] = { unwindAction.destroyExceptionValue() };
						function.getBuilder().CreateCall(getExceptionFreeFunction(function.module()), values);
					}
					return;
				}
				default:
					llvm_unreachable("Unknown unwind action kind.");
			}
		}
		
		bool anyUnwindCleanupActions(Function& function, UnwindState unwindState) {
			const auto& unwindStack = function.unwindStack();
			
			assert(unwindStack.front().isFunctionMarker());
			
			for (size_t i = 0; i < unwindStack.size() - 1; i++) {
				// Perform actions in reverse order (i.e. as a stack).
				const size_t pos = unwindStack.size() - i - 1;
				const auto& unwindAction = unwindStack.at(pos);
				
				if (unwindAction.isActiveForState(unwindState)) {
					return !unwindAction.isTerminator();
				}
			}
			
			return false;
		}
		
		bool anyUnwindActions(Function& function, UnwindState unwindState) {
			const auto& unwindStack = function.unwindStack();
			
			assert(unwindStack.front().isFunctionMarker());
			
			// Ignore top level Function marker.
			for (size_t i = 0; i < unwindStack.size() - 1; i++) {
				// Perform actions in reverse order (i.e. as a stack).
				const size_t pos = unwindStack.size() - i - 1;
				const auto& unwindAction = unwindStack.at(pos);
				
				if (unwindAction.isActiveForState(unwindState)) {
					return true;
				}
			}
			
			return false;
		}
		
		size_t unwindStartPosition(Function& function, UnwindState unwindState) {
			const auto& unwindStack = function.unwindStack();
			
			for (size_t i = 0; i < unwindStack.size(); i++) {
				// Perform actions in reverse order (i.e. as a stack).
				const size_t pos = unwindStack.size() - i - 1;
				auto& unwindAction = unwindStack.at(pos);
				
				if (unwindAction.isActiveForState(unwindState) && unwindAction.isTerminator()) {
					return pos;
				}
			}
			
			llvm_unreachable("Couldn't find unwind terminator action.");
		}
		
		llvm::BasicBlock* genTopUnwindAction(Function& function, size_t position, UnwindState unwindState) {
			auto& unwindStack = function.unwindStack();
			
			auto& topUnwindAction = unwindStack.at(position);
			assert(topUnwindAction.isTerminator());
			
			const auto existingTopBB = topUnwindAction.actionBlock(unwindState);
			if (existingTopBB != nullptr) {
				return existingTopBB;
			}
			
			const auto actionBB = function.createBasicBlock("");
			performScopeExitAction(function, position, unwindState);
			return actionBB;
		}
		
		llvm::BasicBlock* genUnwindBlock(Function& function, UnwindState unwindState) {
			auto& module = function.module();
			auto& unwindStack = function.unwindStack();
			
			const auto currentBB = function.getBuilder().GetInsertBlock();
			
			const auto startPosition = unwindStartPosition(function, unwindState);
			llvm::BasicBlock* nextBB = genTopUnwindAction(function, startPosition, unwindState);
			
			for (size_t i = startPosition + 1; i < unwindStack.size(); i++) {
				auto& unwindAction = unwindStack.at(i);
				
				if (!unwindAction.isActiveForState(unwindState)) {
					// Not relevant to this unwind state.
					continue;
				}
				
				assert(!unwindAction.isTerminator());
				
				const auto existingBB = unwindAction.actionBlock(unwindState);
				
				if (existingBB != nullptr) {
					if (!unwindAction.hasSuccessor(nextBB)) {
						llvm::SwitchInst* switchInst = nullptr;
						
						llvm::Instruction& lastInstruction = existingBB->back();
						if (llvm::isa<llvm::BranchInst>(&lastInstruction)) {
							// Replace the existing branch with a switch.
							const auto branchInst = llvm::cast<llvm::BranchInst>(&lastInstruction);
							branchInst->dump();
							assert(branchInst->isUnconditional());
							
							function.selectBasicBlock(branchInst->getParent());
							const auto currentUnwindStateValue = function.getBuilder().CreateLoad(function.unwindState());
							switchInst = function.getBuilder().CreateSwitch(currentUnwindStateValue, branchInst->getSuccessor(0));
							branchInst->eraseFromParent();
						} else {
							// There's already a switch.
							assert(llvm::isa<llvm::SwitchInst>(&lastInstruction));
							switchInst = llvm::cast<llvm::SwitchInst>(&lastInstruction);
						}
						
						switchInst->addCase(getUnwindStateValue(module, unwindState), nextBB);
						unwindAction.addSuccessor(nextBB);
					}
					
					nextBB = existingBB;
				} else {
					const auto actionBB = function.createBasicBlock("");
					function.selectBasicBlock(actionBB);
					performScopeExitAction(function, i, unwindState);
					function.getBuilder().CreateBr(nextBB);
					
					unwindAction.setActionBlock(unwindState, actionBB);
					unwindAction.addSuccessor(nextBB);
					
					nextBB = actionBB;
				}
			}
			
			function.selectBasicBlock(currentBB);
			
			return nextBB;
		}
		
		void genUnwind(Function& function, UnwindState unwindState) {
			assert(anyUnwindActions(function, unwindState));
			
			if (unwindState != UnwindStateNormal && unwindState != UnwindStateThrow) {
				setCurrentUnwindState(function, unwindState);
			}
			
			function.getBuilder().CreateBr(genUnwindBlock(function, unwindState));
		}
		
		ScopeLifetime::ScopeLifetime(Function& function)
			: function_(function),
			scopeEndBB_(function.createBasicBlock("")) {
				function_.pushUnwindAction(UnwindAction::ScopeMarker(scopeEndBB_));
			}
		
		ScopeLifetime::~ScopeLifetime() {
			if (!lastInstructionTerminates(function_) && anyUnwindActions(function_, UnwindStateNormal)) {
				genUnwind(function_, UnwindStateNormal);
				function_.selectBasicBlock(scopeEndBB_);
			} else {
				scopeEndBB_->eraseFromParent();
			}
			popScope(function_);
		}
		
	}
	
}

