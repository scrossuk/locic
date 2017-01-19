#include <stdexcept>
#include <vector>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Exception.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/ScopeEmitter.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Value* getIsCurrentUnwindState(Function& function, UnwindState state) {
			IREmitter irEmitter(function);
			const auto currentUnwindStateValue = irEmitter.emitRawLoad(function.unwindState(),
			                                                           irEmitter.typeGenerator().getI8Type());
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
			IREmitter irEmitter(function);
			irEmitter.emitRawStore(stateValue, function.unwindState());
		}
		
		llvm::ConstantInt* getUnwindStateValue(Module& module, UnwindState state) {
			return ConstantGenerator(module).getI8(state);
		}
		
		void performScopeExitAction(Function& function, const size_t position, const UnwindState unwindState) {
			const auto& unwindAction = function.unwindStack().at(position);
			
			if (!unwindAction.isActiveForState(unwindState)) {
				return;
			}
			
			IREmitter irEmitter(function);
			
			switch (unwindAction.kind()) {
				case UnwindAction::DESTRUCTOR: {
					irEmitter.emitDestructorCall(unwindAction.destructorValue(),
					                             unwindAction.destructorType());
					return;
				}
				case UnwindAction::CATCH: {
					switch (unwindState) {
						case UnwindStateThrow: {
							irEmitter.emitBranch(unwindAction.catchBlock());
							break;
						}
						default:
							llvm_unreachable("Invalid unwind state for catch action.");
					}
					return;
				}
				case UnwindAction::SCOPEMARKER: {
					switch (unwindState) {
						case UnwindStateNormal: {
							irEmitter.emitBranch(unwindAction.scopeEndBlock());
							break;
						}
						default:
							llvm_unreachable("Invalid unwind state for scope marker action.");
					}
					return;
				}
				case UnwindAction::FUNCTIONMARKER: {
					switch (unwindState) {
						case UnwindStateThrow: {
							TypeGenerator typeGen(function.module());
							const auto exceptionInfoType = typeGen.getStructType(std::vector<llvm::Type*> {typeGen.getPtrType(), typeGen.getI32Type()});
							const auto exceptionInfo = irEmitter.emitRawLoad(function.exceptionInfo(),
							                                                 exceptionInfoType);
							function.getBuilder().CreateResume(exceptionInfo);
							break;
						}
						case UnwindStateReturn: {
							const auto returnType = function.getArgInfo().returnType();
							if (function.getArgInfo().hasReturnVarArgument() || returnType.isVoid()) {
								irEmitter.emitReturnVoid();
							} else {
								function.returnValue(function.getRawReturnValue());
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
							setCurrentUnwindState(function, UnwindStateNormal);
							irEmitter.emitBranch(unwindAction.breakBlock());
							break;
						case UnwindStateContinue:
							setCurrentUnwindState(function, UnwindStateNormal);
							irEmitter.emitBranch(unwindAction.continueBlock());
							break;
						default:
							llvm_unreachable("Invalid unwind state for control flow action.");
					}
					return;
				}
				case UnwindAction::SCOPEEXIT: {
					function.pushUnwindStack(position);
					ScopeEmitter(irEmitter).emitScope(*(unwindAction.scopeExitScope()));
					function.popUnwindStack();
					return;
				}
				case UnwindAction::RETHROWSCOPE: {
					function.pushUnwindStack(position);
					genUnwind(function, UnwindStateThrow);
					function.popUnwindStack();
					return;
				}
				case UnwindAction::DESTROYEXCEPTION: {
					switch (unwindState) {
						case UnwindStateThrow: {
							llvm::Value* const values[] = { unwindAction.destroyExceptionValue() };
							const auto exceptionFreeFunction = getExceptionFreeFunction(function.module());
							const auto callInst = irEmitter.emitCall(exceptionFreeFunction->getFunctionType(),
							                                         exceptionFreeFunction, values);
							callInst->setDoesNotThrow();
							break;
						}
						default:
							llvm_unreachable("Invalid unwind state for destroy exception action.");
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
			
			// Ignore top level Function marker.
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
		
		bool anyUnwindRethrowActions(Function& function) {
			const auto& unwindStack = function.unwindStack();
			
			assert(unwindStack.front().isFunctionMarker());
			
			UnwindState unwindState = UnwindStateRethrow;
			
			// Ignore top level Function marker.
			for (size_t i = 0; i < unwindStack.size() - 1; i++) {
				// Perform actions in reverse order (i.e. as a stack).
				const size_t pos = unwindStack.size() - i - 1;
				const auto& unwindAction = unwindStack.at(pos);
				
				if (unwindAction.isActiveForState(unwindState)) {
					// The rethrow scope (switching the unwind state from
					// 'rethrow' to 'throw) doesn't need to be run if there
					// are no further actions for the 'throw' state, so switch
					// the state to 'throw' and keep looking.
					if (unwindAction.isTerminator() && unwindState == UnwindStateRethrow) {
						assert(unwindAction.isRethrowScope());
						unwindState = UnwindStateThrow;
					} else {
						return true;
					}
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
		
		size_t unwindStartPosition(Function& function, const UnwindState unwindState) {
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
		
		llvm::BasicBlock* genTopUnwindAction(Function& function, const size_t position, const UnwindState unwindState) {
			auto& unwindStack = function.unwindStack();
			
			auto& topUnwindAction = unwindStack.at(position);
			assert(topUnwindAction.isTerminator());
			
			IREmitter irEmitter(function);
			
			const auto existingTopBB = topUnwindAction.actionBlock(unwindState);
			assert(existingTopBB.end == nullptr);
			if (existingTopBB.start != nullptr) {
				return existingTopBB.start;
			}
			
			const auto actionBB = irEmitter.createBasicBlock("");
			irEmitter.selectBasicBlock(actionBB);
			performScopeExitAction(function, position, unwindState);
			assert(irEmitter.lastInstructionTerminates());
			topUnwindAction.setActionBlock(unwindState, BasicBlockRange(actionBB, nullptr));
			return actionBB;
		}
		
		llvm::BasicBlock* genUnwindBlock(Function& function, const UnwindState unwindState) {
			auto& module = function.module();
			auto& unwindStack = function.unwindStack();
			
			IREmitter irEmitter(function);
			
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
				
				const auto existingBBRange = unwindAction.actionBlock(unwindState);
				
				if (existingBBRange.start != nullptr) {
					assert(existingBBRange.end != nullptr);
					
					llvm::Instruction& lastInstruction = existingBBRange.end->back();
					if (llvm::isa<llvm::BranchInst>(&lastInstruction)) {
						const auto branchInst = llvm::cast<llvm::BranchInst>(&lastInstruction);
						assert(branchInst->isUnconditional());
						
						// Only modify this branch if it isn't already
						// pointing to the right place.
						if (branchInst->getSuccessor(0) != nextBB) {
							// Replace the existing branch with a switch.
							irEmitter.selectBasicBlock(branchInst->getParent());
							const auto currentUnwindStateValue = irEmitter.emitRawLoad(function.unwindState(),
							                                                           TypeGenerator(module).getI8Type());
							const auto switchInst = function.getBuilder().CreateSwitch(currentUnwindStateValue, branchInst->getSuccessor(0));
							branchInst->eraseFromParent();
							switchInst->addCase(getUnwindStateValue(module, unwindState), nextBB);
						}
					} else {
						// There's already a switch.
						assert(llvm::isa<llvm::SwitchInst>(&lastInstruction));
						const auto switchInst = llvm::cast<llvm::SwitchInst>(&lastInstruction);
						switchInst->addCase(getUnwindStateValue(module, unwindState), nextBB);
					}
					
					nextBB = existingBBRange.start;
				} else {
					const auto actionStartBB = irEmitter.createBasicBlock("");
					irEmitter.selectBasicBlock(actionStartBB);
					
					performScopeExitAction(function, i, unwindState);
					
					const auto actionEndBB = function.getBuilder().GetInsertBlock();
					
					if (!irEmitter.lastInstructionTerminates()) {
						irEmitter.emitBranch(nextBB);
					}
					
					unwindAction.setActionBlock(unwindState, BasicBlockRange(actionStartBB, actionEndBB));
					
					nextBB = actionStartBB;
				}
			}
			
			irEmitter.selectBasicBlock(currentBB);
			
			return nextBB;
		}
		
		void genUnwind(Function& function, UnwindState unwindState) {
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
			if (!function_.lastInstructionTerminates() && anyUnwindCleanupActions(function_, UnwindStateNormal)) {
				genUnwind(function_, UnwindStateNormal);
				function_.selectBasicBlock(scopeEndBB_);
			} else {
				scopeEndBB_->eraseFromParent();
			}
			
			const auto& unwindStack = function_.unwindStack();
			while (!unwindStack.empty()) {
				const bool isMarker = unwindStack.back().isScopeMarker();
				function_.popUnwindAction();
				
				if (isMarker) {
					return;
				}
			}
			
			llvm_unreachable("Scope marker not found.");
		}
		
	}
	
}

