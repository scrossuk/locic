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
				} else if (unwindAction.isCatchBlock()) {
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
			
			size_t getScopeLevel(const UnwindStack& unwindStack) {
				for (size_t i = 0; i < unwindStack.size(); i++) {
					const size_t pos = unwindStack.size() - i - 1;
					const auto& unwindAction = unwindStack.at(pos);
					if (unwindAction.isScopeMarker()) {
						return pos;
					}
				}
				llvm_unreachable("Scope marker not found.");
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
			
		}
		
		void performScopeExitAction(Function& function, size_t position, bool isExceptionState, bool isRethrow) {
			const auto& unwindAction = function.unwindStack().at(position);
			
			if (!isActiveAction(unwindAction, isExceptionState, isRethrow)) {
				return;
			}
			
			if (unwindAction.isDestructor()) {
				genDestructorCall(function, unwindAction.destroyType(), unwindAction.destroyValue());
			} else if (unwindAction.isScopeExit()) {
				function.pushUnwindStack(position);
				genScope(function, *(unwindAction.scopeExitScope()));
				function.popUnwindStack();
			} else if (unwindAction.isCatchBlock()) {
				function.getBuilder().CreateCall(getExceptionFreeFunction(function.module()), std::vector<llvm::Value*>{ unwindAction.catchExceptionValue() });
			}
		}
		
		void genScopeExitActions(Function& function, bool isExceptionState, bool isRethrow) {
			const auto& unwindStack = function.unwindStack();
			
			// Only generate this code if there are actually actions to perform.
			if (!scopeHasExitAction(function, isExceptionState, isRethrow)) return;
			
			const auto scopeLevel = getScopeLevel(unwindStack);
			
			// Create a new basic block to make this clearer...
			const auto scopeDestroyStartBB = function.createBasicBlock(makeString("scopeExitActions_%llu_START", (unsigned long long) scopeLevel));
			function.getBuilder().CreateBr(scopeDestroyStartBB);
			function.selectBasicBlock(scopeDestroyStartBB);
			
			for (size_t i = 0; i < unwindStack.size(); i++) {
				// Perform actions in reverse order (i.e. as a stack).
				const size_t pos = unwindStack.size() - i - 1;
				const auto& unwindAction = unwindStack.at(pos);
				if (unwindAction.isScopeMarker()) break;
				
				performScopeExitAction(function, pos, isExceptionState, isRethrow);
			}
			
			// ...and another to make it clear where it ends.
			const auto scopeDestroyEndBB = function.createBasicBlock(makeString("scopeExitActions_%llu_END", (unsigned long long) scopeLevel));
			function.getBuilder().CreateBr(scopeDestroyEndBB);
			function.selectBasicBlock(scopeDestroyEndBB);
		}
		
		void genStatementExitActions(Function& function, bool isExceptionState, bool isRethrow) {
			const auto& unwindStack = function.unwindStack();
			
			// Only generate this code if there are actually actions to perform.
			if (!statementHasExitAction(function, isExceptionState, isRethrow)) return;
			
			// Create a new basic block to make this clearer...
			const auto statementDestroyStartBB = function.createBasicBlock("statementExitActions_START");
			function.getBuilder().CreateBr(statementDestroyStartBB);
			function.selectBasicBlock(statementDestroyStartBB);
			
			for (size_t i = 0; i < unwindStack.size(); i++) {
				// Perform actions in reverse order (i.e. as a stack).
				const size_t pos = unwindStack.size() - i - 1;
				const auto& unwindAction = unwindStack.at(pos);
				if (unwindAction.isStatementMarker()) break;
				
				performScopeExitAction(function, pos, isExceptionState, isRethrow);
			}
			
			// ...and another to make it clear where it ends.
			const auto statementDestroyEndBB = function.createBasicBlock("statementExitActions_END");
			function.getBuilder().CreateBr(statementDestroyEndBB);
			function.selectBasicBlock(statementDestroyEndBB);
		}
		
		void genAllScopeExitActions(Function& function, bool isExceptionState, bool isRethrow) {
			const auto& unwindStack = function.unwindStack();
			
			// Create a new basic block to make this clearer.
			const auto scopeDestroyAllBB = function.createBasicBlock("exitAllScopes");
			function.getBuilder().CreateBr(scopeDestroyAllBB);
			function.selectBasicBlock(scopeDestroyAllBB);
			
			for (size_t i = 0; i < unwindStack.size(); i++) {
				// Perform actions in reverse order (i.e. as a stack).
				const size_t pos = unwindStack.size() - i - 1;
				performScopeExitAction(function, pos, isExceptionState, isRethrow);
			}
		}
		
		ScopeLifetime::ScopeLifetime(Function& function)
			: function_(function) {
			const size_t scopeLevel = function_.unwindStack().size();
			const auto scopeStartBB = function_.createBasicBlock(makeString("scope_%llu_START", (unsigned long long) scopeLevel));
			function_.getBuilder().CreateBr(scopeStartBB);
			function_.selectBasicBlock(scopeStartBB);
			
			function_.unwindStack().push_back(UnwindAction::ScopeMarker());
		}
		
		ScopeLifetime::~ScopeLifetime() {
			const bool isExceptionState = false;
			const bool isRethrow = false;
			genScopeExitActions(function_, isExceptionState, isRethrow);
			popScope(function_.unwindStack());
			
			const size_t scopeLevel = function_.unwindStack().size();
			const auto scopeEndBB = function_.createBasicBlock(makeString("scope_%llu_END", (unsigned long long) scopeLevel));
			function_.getBuilder().CreateBr(scopeEndBB);
			function_.selectBasicBlock(scopeEndBB);
		}
		
		StatementLifetime::StatementLifetime(Function& function)
			: function_(function) {
			function_.unwindStack().push_back(UnwindAction::StatementMarker());
		}
		
		StatementLifetime::~StatementLifetime() {
			const bool isExceptionState = false;
			const bool isRethrow = false;
			genStatementExitActions(function_, isExceptionState, isRethrow);
			popStatement(function_.unwindStack());
		}
		
	}
	
}

