#include <locic/SEM.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		UnwindAction UnwindAction::Destructor(SEM::Type* type, llvm::Value* value) {
			UnwindAction action(UnwindAction::DESTRUCTOR);
			action.actions_.destructorAction.type = type;
			action.actions_.destructorAction.value = value;
			return action;
		}
		
		UnwindAction UnwindAction::CatchException(llvm::BasicBlock* catchBlock, llvm::Constant* catchTypeInfo) {
			UnwindAction action(UnwindAction::CATCH);
			action.actions_.catchAction.block = catchBlock;
			action.actions_.catchAction.typeInfo = catchTypeInfo;
			return action;
		}
		
		UnwindAction UnwindAction::ScopeMarker(llvm::BasicBlock* scopeEndBlock) {
			UnwindAction action(UnwindAction::SCOPEMARKER);
			action.actions_.scopeAction.block = scopeEndBlock;
			return action;
		}
		
		UnwindAction UnwindAction::FunctionMarker() {
			return UnwindAction(UnwindAction::FUNCTIONMARKER);
		}
		
		UnwindAction UnwindAction::ControlFlow(llvm::BasicBlock* breakBlock, llvm::BasicBlock* continueBlock) {
			UnwindAction action(UnwindAction::CONTROLFLOW);
			action.actions_.controlFlowAction.breakBlock = breakBlock;
			action.actions_.controlFlowAction.continueBlock = continueBlock;
			return action;
		}
		
		UnwindAction UnwindAction::ScopeExit(ScopeExitState state, SEM::Scope* scope) {
			UnwindAction action(UnwindAction::SCOPEEXIT);
			action.actions_.scopeExitAction.state = state;
			action.actions_.scopeExitAction.scope = scope;
			return action;
		}
		
		UnwindAction UnwindAction::RethrowScope() {
			return UnwindAction(UnwindAction::RETHROWSCOPE);
		}
		
		UnwindAction UnwindAction::DestroyException(llvm::Value* exceptionValue) {
			UnwindAction action(UnwindAction::DESTROYEXCEPTION);
			action.actions_.destroyExceptionAction.exceptionValue = exceptionValue;
			return action;
		}
		
		UnwindAction::Kind UnwindAction::kind() const {
			return kind_;
		}
		
		bool UnwindAction::isDestructor() const {
			return kind() == UnwindAction::DESTRUCTOR;
		}
		
		bool UnwindAction::isCatch() const {
			return kind() == UnwindAction::CATCH;
		}
		
		bool UnwindAction::isScopeMarker() const {
			return kind() == UnwindAction::SCOPEMARKER;
		}
		
		bool UnwindAction::isFunctionMarker() const {
			return kind() == UnwindAction::FUNCTIONMARKER;
		}
		
		bool UnwindAction::isControlFlow() const {
			return kind() == UnwindAction::CONTROLFLOW;
		}
		
		bool UnwindAction::isScopeExit() const {
			return kind() == UnwindAction::SCOPEEXIT;
		}
		
		bool UnwindAction::isRethrowScope() const {
			return kind() == UnwindAction::RETHROWSCOPE;
		}
		
		bool UnwindAction::isDestroyException() const {
			return kind() == UnwindAction::DESTROYEXCEPTION;
		}
		
		SEM::Type* UnwindAction::destructorType() const {
			assert(isDestructor());
			return actions_.destructorAction.type;
		}
		
		llvm::Value* UnwindAction::destructorValue() const {
			assert(isDestructor());
			return actions_.destructorAction.value;
		}
		
		llvm::BasicBlock* UnwindAction::catchBlock() const {
			assert(isCatch());
			return actions_.catchAction.block;
		}
		
		llvm::Constant* UnwindAction::catchTypeInfo() const {
			assert(isCatch());
			return actions_.catchAction.typeInfo;
		}
		
		llvm::BasicBlock* UnwindAction::scopeEndBlock() const {
			assert(isScopeMarker());
			return actions_.scopeAction.block;
		}
		
		llvm::BasicBlock* UnwindAction::breakBlock() const {
			assert(isControlFlow());
			return actions_.controlFlowAction.breakBlock;
		}
		
		llvm::BasicBlock* UnwindAction::continueBlock() const {
			assert(isControlFlow());
			return actions_.controlFlowAction.continueBlock;
		}
		
		ScopeExitState UnwindAction::scopeExitState() const {
			assert(isScopeExit());
			return actions_.scopeExitAction.state;
		}
		
		SEM::Scope* UnwindAction::scopeExitScope() const {
			assert(isScopeExit());
			return actions_.scopeExitAction.scope;
		}
		
		llvm::Value* UnwindAction::destroyExceptionValue() const {
			assert(isDestroyException());
			return actions_.destroyExceptionAction.exceptionValue;
		}
		
		bool UnwindAction::isTerminator() const {
			switch (kind()) {
				case CATCH:
				case SCOPEMARKER:
				case CONTROLFLOW:
				case FUNCTIONMARKER:
				case RETHROWSCOPE:
					return true;
				case DESTRUCTOR:
				case SCOPEEXIT:
				case DESTROYEXCEPTION:
					return false;
				default:
					llvm_unreachable("Unknown unwind action kind.");
			}
		}
		
		bool UnwindAction::isActiveForState(UnwindState unwindState) const {
			switch (kind()) {
				case DESTRUCTOR: {
					// Destructors are always executed when exiting a scope.
					return true;
				}
				case CATCH: {
					return unwindState == UnwindStateThrow;
				}
				case SCOPEMARKER: {
					return unwindState == UnwindStateNormal;
				}
				case FUNCTIONMARKER: {
					return unwindState == UnwindStateThrow || unwindState == UnwindStateReturn;
				}
				case CONTROLFLOW: {
					return unwindState == UnwindStateBreak || unwindState == UnwindStateContinue;
				}
				case SCOPEEXIT: {
					// Scope exit actions may only be executed on success/failure.
					const bool isExceptionState = (unwindState == UnwindStateThrow || unwindState == UnwindStateRethrow);
					const bool supportsExceptionState = (scopeExitState() != SCOPEEXIT_SUCCESS);
					const bool supportsNormalState = (scopeExitState() != SCOPEEXIT_FAILURE);
					return (isExceptionState && supportsExceptionState) || (!isExceptionState && supportsNormalState);
				}
				case RETHROWSCOPE: {
					return unwindState == UnwindStateRethrow;
				}
				case DESTROYEXCEPTION: {
					return unwindState == UnwindStateThrow;
				}
				default:
					llvm_unreachable("Unknown unwind action kind.");
			}
		}
		
		BasicBlockRange UnwindAction::actionBlock(UnwindState state) {
			return actionBB_[state];
		}
		
		void UnwindAction::setActionBlock(UnwindState state, BasicBlockRange actionBB) {
			switch (kind()) {
				case DESTRUCTOR: {
					if (state == UnwindStateThrow || state == UnwindStateRethrow) {
						actionBB_[UnwindStateThrow] = actionBB;
						actionBB_[UnwindStateRethrow] = actionBB;
					} else {
						actionBB_[UnwindStateNormal] = actionBB;
						actionBB_[UnwindStateReturn] = actionBB;
						actionBB_[UnwindStateBreak] = actionBB;
						actionBB_[UnwindStateContinue] = actionBB;
					}
					break;
				}
				case CATCH: {
					assert(state == UnwindStateThrow || state == UnwindStateRethrow);
					actionBB_[UnwindStateThrow] = actionBB;
					actionBB_[UnwindStateRethrow] = actionBB;
					break;
				}
				case SCOPEMARKER: {
					assert(state == UnwindStateNormal);
					actionBB_[UnwindStateNormal] = actionBB;
					break;
				}
				case FUNCTIONMARKER: {
					assert(state == UnwindStateThrow || state == UnwindStateReturn);
					if (state == UnwindStateThrow) {
						actionBB_[UnwindStateThrow] = actionBB;
					} else {
						actionBB_[UnwindStateReturn] = actionBB;
					}
					break;
				}
				case CONTROLFLOW: {
					assert(state == UnwindStateBreak || state == UnwindStateContinue);
					if (state == UnwindStateBreak) {
						actionBB_[UnwindStateBreak] = actionBB;
					} else {
						actionBB_[UnwindStateContinue] = actionBB;
					}
					break;
				}
				case SCOPEEXIT: {
					if (state == UnwindStateThrow || state == UnwindStateRethrow) {
						actionBB_[UnwindStateThrow] = actionBB;
						actionBB_[UnwindStateRethrow] = actionBB;
					} else {
						actionBB_[UnwindStateNormal] = actionBB;
						actionBB_[UnwindStateReturn] = actionBB;
						actionBB_[UnwindStateBreak] = actionBB;
						actionBB_[UnwindStateContinue] = actionBB;
					}
					break;
				}
				case RETHROWSCOPE: {
					assert(state == UnwindStateRethrow);
					actionBB_[UnwindStateRethrow] = actionBB;
					break;
				}
				case DESTROYEXCEPTION: {
					assert(state != UnwindStateRethrow);
					if (state == UnwindStateThrow) {
						actionBB_[UnwindStateThrow] = actionBB;
					} else {
						actionBB_[UnwindStateNormal] = actionBB;
						actionBB_[UnwindStateReturn] = actionBB;
						actionBB_[UnwindStateBreak] = actionBB;
						actionBB_[UnwindStateContinue] = actionBB;
					}
					break;
				}
				default:
					llvm_unreachable("Unknown unwind action kind.");
			}
		}
		
		llvm::BasicBlock* UnwindAction::landingPadBlock() const {
			return landingPadBB_;
		}
		
		void UnwindAction::setLandingPadBlock(llvm::BasicBlock* landingPadBB) {
			landingPadBB_ = landingPadBB;
		}
		
	}
	
}

