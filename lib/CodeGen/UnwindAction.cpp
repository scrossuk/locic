#include <locic/SEM.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		UnwindAction UnwindAction::Destroy(SEM::Type* type, llvm::Value* value) {
			UnwindAction action(UnwindAction::DESTRUCTOR);
			action.actions_.destroyAction.type = type;
			action.actions_.destroyAction.value = value;
			return action;
		}
		
		UnwindAction UnwindAction::CatchException(llvm::BasicBlock* catchBlock, llvm::Constant* catchTypeInfo) {
			UnwindAction action(UnwindAction::CATCH);
			action.actions_.catchAction.block = catchBlock;
			action.actions_.catchAction.typeInfo = catchTypeInfo;
			return action;
		}
		
		UnwindAction UnwindAction::ScopeMarker() {
			return UnwindAction(UnwindAction::SCOPEMARKER);
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
		
		UnwindAction UnwindAction::CatchBlock(llvm::Value* exceptionValue) {
			UnwindAction action(UnwindAction::CATCHBLOCK);
			action.actions_.catchBlock.exceptionValue = exceptionValue;
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
		
		bool UnwindAction::isControlFlow() const {
			return kind() == UnwindAction::CONTROLFLOW;
		}
		
		bool UnwindAction::isScopeExit() const {
			return kind() == UnwindAction::SCOPEEXIT;
		}
		
		bool UnwindAction::isCatchBlock() const {
			return kind() == UnwindAction::CATCHBLOCK;
		}
		
		SEM::Type* UnwindAction::destroyType() const {
			assert(isDestructor());
			return actions_.destroyAction.type;
		}
		
		llvm::Value* UnwindAction::destroyValue() const {
			assert(isDestructor());
			return actions_.destroyAction.value;
		}
		
		llvm::BasicBlock* UnwindAction::catchBlock() const {
			assert(isCatch());
			return actions_.catchAction.block;
		}
		
		llvm::Constant* UnwindAction::catchTypeInfo() const {
			assert(isCatch());
			return actions_.catchAction.typeInfo;
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
		
		llvm::Value* UnwindAction::catchExceptionValue() const {
			assert(isCatchBlock());
			return actions_.catchBlock.exceptionValue;
		}
		
	}
	
}

