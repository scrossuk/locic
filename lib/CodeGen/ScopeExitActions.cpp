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
		
		llvm::Value* getUnwindStateValue(Module& module, UnwindState state) {
			return ConstantGenerator(module).getI8(state);
		}
		
		namespace {
		
			bool isActiveAction(const UnwindAction& unwindAction, bool isExceptionState, bool isRethrow) {
				assert(isExceptionState || !isRethrow);
				
				if (unwindAction.isDestructor()) {
					// Destructors are always executed when exiting a scope.
					return true;
				} else if (unwindAction.isScopeExit()) {
					// Scope exit actions may only be executed on success/failure.
					const bool supportsExceptionState = (unwindAction.scopeExitState() != SCOPEEXIT_SUCCESS);
					const bool supportsNormalState = (unwindAction.scopeExitState() != SCOPEEXIT_FAILURE);
					return (isExceptionState && supportsExceptionState) || (!isExceptionState && supportsNormalState);
				} else if (unwindAction.isDestroyException()) {
					return !isRethrow;
				} else {
					// Everything else is just a placeholder so doesn't
					// actually generate code when leaving a scope.
					return false;
				}
			}
			
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
			
			void popScope(UnwindStack& unwindStack) {
				while (!unwindStack.empty()) {
					const bool isMarker = unwindStack.back().isScopeMarker();
					unwindStack.pop_back();
					
					if (isMarker) {
						return;
					}
				}
				
				llvm_unreachable("Scope marker not found.");
			}
			
			void popStatement(UnwindStack& unwindStack) {
				while (!unwindStack.empty()) {
					const bool isMarker = unwindStack.back().isStatementMarker();
					unwindStack.pop_back();
					
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
		
		void performScopeExitAction(Function& function, size_t position, bool isExceptionState, bool isRethrow) {
			const auto& unwindAction = function.unwindStack().at(position);
			
			if (!isActiveAction(unwindAction, isExceptionState, isRethrow)) {
				return;
			}
			
			if (unwindAction.isDestructor()) {
				genDestructorCall(function, unwindAction.destructorType(), unwindAction.destructorValue());
			} else if (unwindAction.isScopeExit()) {
				function.pushUnwindStack(position);
				genScope(function, *(unwindAction.scopeExitScope()));
				function.popUnwindStack();
				
				if (lastInstructionTerminates(function)) {
					function.selectBasicBlock(function.createBasicBlock(""));
				}
			} else if (unwindAction.isDestroyException()) {
				llvm::Value* const values[] = { unwindAction.destroyExceptionValue() };
				function.getBuilder().CreateCall(getExceptionFreeFunction(function.module()), values);
			}
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
			function_.unwindStack().push_back(UnwindAction::ScopeMarker());
		}
		
		ScopeLifetime::~ScopeLifetime() {
			if (!lastInstructionTerminates(function_)) {
				const bool isExceptionState = false;
				const bool isRethrow = false;
				genScopeExitActions(function_, isExceptionState, isRethrow);
			}
			popScope(function_.unwindStack());
		}
		
		StatementLifetime::StatementLifetime(Function& function)
			: function_(function) {
			function_.unwindStack().push_back(UnwindAction::StatementMarker());
		}
		
		StatementLifetime::~StatementLifetime() {
			if (!lastInstructionTerminates(function_)) {
				const bool isExceptionState = false;
				const bool isRethrow = false;
				genStatementExitActions(function_, isExceptionState, isRethrow);
			}
			popStatement(function_.unwindStack());
		}
		
	}
	
}

