#include <locic/SEM.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		UnwindAction UnwindAction::Destroy(llvm::BasicBlock* destroyBlock) {
			UnwindAction action(UnwindAction::DESTRUCTOR);
			action.actions_.destroyAction.block = destroyBlock;
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
			action.actions_.scopeMarker.block = scopeEndBlock;
			return action;
		}
		
		UnwindAction UnwindAction::StatementMarker(llvm::BasicBlock* statementEndBlock) {
			UnwindAction action(UnwindAction::STATEMENTMARKER);
			action.actions_.statementMarker.block = statementEndBlock;
			return action;
		}
		
		UnwindAction UnwindAction::ControlFlow(llvm::BasicBlock* actionBlock) {
			UnwindAction action(UnwindAction::CONTROLFLOW);
			action.actions_.controlFlowAction.block = actionBlock;
			return action;
		}
		
		UnwindAction UnwindAction::ScopeExit(llvm::BasicBlock* actionBlock) {
			UnwindAction action(UnwindAction::SCOPEEXIT);
			action.actions_.scopeExitAction.block = actionBlock;
			return action;
		}
		
		UnwindAction UnwindAction::CatchBlock(llvm::BasicBlock* destroyBlock, llvm::Value* exceptionValue) {
			UnwindAction action(UnwindAction::CATCHBLOCK);
			action.actions_.catchBlock.block = destroyBlock;
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
		
		bool UnwindAction::isStatementMarker() const {
			return kind() == UnwindAction::STATEMENTMARKER;
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
		
		llvm::BasicBlock* UnwindAction::destroyBlock() const {
			assert(isDestructor());
			return actions_.destroyAction.block;
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
			return actions_.scopeMarker.block;
		}
		
		llvm::BasicBlock* UnwindAction::statementEndBlock() const {
			assert(isStatementMarker());
			return actions_.statementMarker.block;
		}
		
		llvm::BasicBlock* UnwindAction::controlFlowBlock() const {
			assert(isControlFlow());
			return actions_.controlFlowAction.block;
		}
		
		llvm::BasicBlock* UnwindAction::scopeExitBlock() const {
			assert(isScopeExit());
			return actions_.scopeExitAction.block;
		}
		
		llvm::BasicBlock* UnwindAction::destroyExceptionBlock() const {
			assert(isCatchBlock());
			return actions_.catchBlock.block;
		}
		
		llvm::Value* UnwindAction::catchExceptionValue() const {
			assert(isCatchBlock());
			return actions_.catchBlock.exceptionValue;
		}
		
		llvm::BasicBlock* UnwindAction::unwindBlock() const {
			switch (kind()) {
				case DESTRUCTOR:
					return destroyBlock();
				case CATCH:
					return catchBlock();
				case SCOPEMARKER:
					return scopeEndBlock();
				case STATEMENTMARKER:
					return statementEndBlock();
				case CONTROLFLOW:
					return controlFlowBlock();
				case SCOPEEXIT:
					return scopeExitBlock();
				case CATCHBLOCK:
					return destroyExceptionBlock();
				default:
					return nullptr;
			}
		}
		
	}
	
}

