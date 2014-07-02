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
		
			bool scopeHasExitAction(Function& function, bool isExceptionState, bool isRethrow) {
				const auto& unwindStack = function.unwindStack();
				
				for (size_t i = 0; i < unwindStack.size(); i++) {
					const size_t pos = unwindStack.size() - i - 1;
					const auto& unwindAction = unwindStack.at(pos);
					
					if (unwindAction.isScopeMarker()) {
						return false;
					}
					
					if (isActiveAction(unwindAction, isExceptionState, isRethrow)) {
						return true;
					}
				}
				
				llvm_unreachable("Scope marker not found.");
			}
			
			bool statementHasExitAction(Function& function, bool isExceptionState, bool isRethrow) {
				const auto& unwindStack = function.unwindStack();
				
				for (size_t i = 0; i < unwindStack.size(); i++) {
					const size_t pos = unwindStack.size() - i - 1;
					const auto& unwindAction = unwindStack.at(pos);
					
					if (unwindAction.isStatementMarker()) {
						return false;
					}
					
					if (isActiveAction(unwindAction, isExceptionState, isRethrow)) {
						return true;
					}
				}
				
				llvm_unreachable("Statement marker not found.");
			}
			
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
			
			void popStatement(Function& function) {
				const auto& unwindStack = function.unwindStack();
				while (!unwindStack.empty()) {
					const bool isMarker = unwindStack.back().isStatementMarker();
					function.popUnwindAction();
					
					if (isMarker) {
						return;
					}
				}
				
				llvm_unreachable("Statement marker not found.");
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
		
		bool isActiveAction(const UnwindAction& unwindAction, UnwindState unwindState) {
			const bool isExceptionState = (unwindState == UnwindStateThrow || unwindState == UnwindStateRethrow);
			
			switch (unwindAction.kind()) {
				case UnwindAction::DESTRUCTOR: {
					// Destructors are always executed when exiting a scope.
					return true;
				}
				case UnwindAction::CATCH: {
					return isExceptionState;
				}
				case UnwindAction::SCOPEMARKER: {
					// Just a placeholder.
					return false;
				}
				case UnwindAction::STATEMENTMARKER: {
					// Just a placeholder.
					return false;
				}
				case UnwindAction::CONTROLFLOW: {
					return !isExceptionState;
				}
				case UnwindAction::SCOPEEXIT: {
					const bool isExceptionState = (unwindState == UnwindStateThrow || unwindState == UnwindStateRethrow);
					
					// Scope exit actions may only be executed on success/failure.
					const bool supportsExceptionState = (unwindAction.scopeExitState() != SCOPEEXIT_SUCCESS);
					const bool supportsNormalState = (unwindAction.scopeExitState() != SCOPEEXIT_FAILURE);
					return (isExceptionState && supportsExceptionState) || (!isExceptionState && supportsNormalState);
				}
				case UnwindAction::DESTROYEXCEPTION: {
					return !isRethrow;
				}
				default:
					llvm_unreachable("Unknown unwind action kind.");
			}
		}
		
		bool isTerminatorAction(const UnwindAction& unwindAction, UnwindState unwindState) {
			switch (unwindAction.kind()) {
				case UnwindAction::DESTRUCTOR: {
					return false;
				}
				case UnwindAction::CATCH: {
					return unwindState == UnwindStateThrow || unwindState == UnwindStateRethrow;
				}
				case UnwindAction::SCOPEMARKER: {
					return unwindState == UnwindStateNormal;
				}
				case UnwindAction::STATEMENTMARKER: {
					return unwindState == UnwindStateNormal;
				}
				case UnwindAction::CONTROLFLOW: {
					return unwindState == UnwindStateBreak || unwindState == UnwindStateContinue;
				}
				case UnwindAction::SCOPEEXIT: {
					return false;
				}
				case UnwindAction::DESTROYEXCEPTION: {
					return false;
				}
				default:
					llvm_unreachable("Unknown unwind action kind.");
			}
		}
		
		void performScopeExitAction(Function& function, size_t position, UnwindState unwindState) {
			auto& module = function.module();
			
			const auto& unwindAction = function.unwindStack().at(position);
			
			if (!isActiveAction(unwindAction, isExceptionState, isRethrow)) {
				return;
			}
			
			switch (unwindAction.kind()) {
				case UnwindAction::DESTRUCTOR: {
					genDestructorCall(function, unwindAction.destructorType(), unwindAction.destructorValue());
					return;
				}
				case UnwindAction::CATCH: {
					// TODO...
					return;
				}
				case UnwindAction::SCOPEMARKER: {
					// Just a placeholder.
					return;
				}
				case UnwindAction::STATEMENTMARKER: {
					// Just a placeholder.
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
							break;
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
		
		bool anyUnwindActions(Function& function, UnwindState unwindState) {
			const auto& unwindStack = function.unwindStack();
			
			for (size_t i = 0; i < unwindStack.size(); i++) {
				// Perform actions in reverse order (i.e. as a stack).
				const size_t pos = unwindStack.size() - i - 1;
				auto& unwindAction = unwindStack.at(pos);
				
				if (isActiveAction(unwindAction, unwindState)) {
					return true;
				}
			}
			
			return false;
		}
		
		llvm::BasicBlock* genUnwind(Function& function, UnwindState unwindState) {
			auto& unwindStack = function.unwindStack();
			
			llvm::BasicBlock* initialUnwindBB = nullptr;
			
			for (size_t i = 0; i < unwindStack.size(); i++) {
				// Perform actions in reverse order (i.e. as a stack).
				const size_t pos = unwindStack.size() - i - 1;
				auto& unwindAction = unwindStack.at(pos);
				
				if (!isActiveAction(unwindAction, unwindState)) {
					// Not relevant to this unwind state.
					continue;
				}
				
				const auto existingBB = unwindAction.actionBlock(unwindState);
				const auto unwindBB = existingBB != nullptr ? existingBB : function.createBasicBlock("");
				
				if (initialUnwindBB == nullptr) {
					initialUnwindBB = unwindBB;
				}
				
				if (existingBB == nullptr) {
					function.selectBasicBlock(unwindBB);
					
				}
				
				if (isTerminatorAction(unwindAction, unwindState)) {
					// The last basic block will have been
					// terminated by a branch since this is
					// a terminator (i.e. the end of unwinding).
					return initialUnwindBB;
				}
				
				if (existingBB != nullptr) {
					const auto& generatedStates = unwindAction.generatedStates();
					if (!generatedStates.at(unwindState)) {
						const auto stateUnwindBB = function.createBasicBlock("");
						
						// Unwind action hasn't been generated for this state.
						if (generatedStates.count() > 1) {
							// It's already been generated for multiple states,
							// so there should be a switch.
							assert(!unwindBB->empty());
							assert(llvm::isa<llvm::SwitchInst>(unwindBB->front()));
							auto& switchInst = llvm::cast<llvm::SwitchInst>(unwindBB->front());
							switchInst->
						} else {
							// Action has been generated for just one state,
							// so there should be an unconditional branch.
							assert(generatedStates.any());
							assert(llvm::isa<llvm::BranchInst>(unwindBB->front()));
						}
						
						unwindAction.setGeneratedState(unwindState);
					}
				}
			}
			
			llvm_unreachable("Unwinding reached top of function.");
		}
		
		void genScopeExitActions(Function& function, bool isExceptionState, bool isRethrow) {
			const auto& unwindStack = function.unwindStack();
			
			// Only generate this code if there are actually actions to perform.
			if (!scopeHasExitAction(function, isExceptionState, isRethrow)) {
				return;
			}
			
			for (size_t i = 0; i < unwindStack.size(); i++) {
				// Perform actions in reverse order (i.e. as a stack).
				const size_t pos = unwindStack.size() - i - 1;
				const auto& unwindAction = unwindStack.at(pos);
				
				if (unwindAction.isScopeMarker()) {
					break;
				}
				
				performScopeExitAction(function, pos, isExceptionState, isRethrow);
			}
		}
		
		void genStatementExitActions(Function& function, bool isExceptionState, bool isRethrow) {
			const auto& unwindStack = function.unwindStack();
			
			// Only generate this code if there are actually actions to perform.
			if (!statementHasExitAction(function, isExceptionState, isRethrow)) {
				return;
			}
			
			for (size_t i = 0; i < unwindStack.size(); i++) {
				// Perform actions in reverse order (i.e. as a stack).
				const size_t pos = unwindStack.size() - i - 1;
				const auto& unwindAction = unwindStack.at(pos);
				
				if (unwindAction.isStatementMarker()) {
					break;
				}
				
				performScopeExitAction(function, pos, isExceptionState, isRethrow);
			}
		}
		
		void genAllScopeExitActions(Function& function, bool isExceptionState, bool isRethrow) {
			const auto& unwindStack = function.unwindStack();
			for (size_t i = 0; i < unwindStack.size(); i++) {
				// Perform actions in reverse order (i.e. as a stack).
				const size_t pos = unwindStack.size() - i - 1;
				performScopeExitAction(function, pos, isExceptionState, isRethrow);
			}
		}
		
		ScopeLifetime::ScopeLifetime(Function& function)
			: function_(function) {
			function_.pushUnwindAction(UnwindAction::ScopeMarker());
		}
		
		ScopeLifetime::~ScopeLifetime() {
			if (!lastInstructionTerminates(function_)) {
				const bool isExceptionState = false;
				const bool isRethrow = false;
				genScopeExitActions(function_, isExceptionState, isRethrow);
			}
			popScope(function_);
		}
		
		StatementLifetime::StatementLifetime(Function& function)
			: function_(function) {
			function_.pushUnwindAction(UnwindAction::StatementMarker());
		}
		
		StatementLifetime::~StatementLifetime() {
			if (!lastInstructionTerminates(function_)) {
				const bool isExceptionState = false;
				const bool isRethrow = false;
				genStatementExitActions(function_, isExceptionState, isRethrow);
			}
			popStatement(function_);
		}
		
	}
	
}

