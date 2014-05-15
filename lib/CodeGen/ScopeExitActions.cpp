#include <stdexcept>
#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/SEM.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenStatement.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		namespace {
			
			bool isActiveAction(const UnwindAction& unwindAction, bool isExceptionState) {
				if (unwindAction.isDestructor()) {
					// Destructors are always executed when exiting a scope.
					return true;
				} else if (unwindAction.isScopeExit()) {
					// Scope exit actions may only be executed on success/failure.
					const bool supportsExceptionState = (unwindAction.scopeExitState() != SCOPEEXIT_SUCCESS);
					const bool supportsNormalState = (unwindAction.scopeExitState() != SCOPEEXIT_FAILURE);
					return (isExceptionState && supportsExceptionState) || (!isExceptionState && supportsNormalState);
				} else {
					// Everything else is just a placeholder so doesn't
					// actually generate code when leaving a scope.
					return false;
				}
			}
			
			bool scopeHasExitAction(Function& function, bool isExceptionState) {
				const auto& unwindStack = function.unwindStack();
				
				for (size_t i = 0; i < unwindStack.size(); i++) {
					const size_t pos = unwindStack.size() - i - 1;
					const auto& unwindAction = unwindStack.at(pos);
					if (unwindAction.isScopeMarker()) {
						return false;
					}
					
					if (isActiveAction(unwindAction, isExceptionState)) {
						return true;
					}
				}
				
				throw std::runtime_error("Scope marker not found.");
			}
			
			size_t getScopeLevel(const UnwindStack& unwindStack) {
				for (size_t i = 0; i < unwindStack.size(); i++) {
					const size_t pos = unwindStack.size() - i - 1;
					const auto& unwindAction = unwindStack.at(pos);
					if (unwindAction.isScopeMarker()) {
						return pos;
					}
				}
				throw std::runtime_error("Scope marker not found.");
			}
			
			void popScope(UnwindStack& unwindStack) {
				while (!unwindStack.empty()) {
					const bool isMarker = unwindStack.back().isScopeMarker();
					unwindStack.pop_back();
					if (isMarker) {
						return;
					}
				}
				throw std::runtime_error("Scope marker not found.");
			}
			
		}
		
		void performScopeExitAction(Function& function, size_t position, bool isExceptionState) {
			const auto& unwindAction = function.unwindStack().at(position);
			
			if (!isActiveAction(unwindAction, isExceptionState)) {
				return;
			}
			
			if (unwindAction.isDestructor()) {
				genDestructorCall(function, unwindAction.destroyType(), unwindAction.destroyValue());
			} else if (unwindAction.isScopeExit()) {
				function.pushUnwindStack(position);
				genScope(function, *(unwindAction.scopeExitScope()));
				function.popUnwindStack();
			}
		}
		
		void genScopeExitActions(Function& function, bool isExceptionState) {
			const auto& unwindStack = function.unwindStack();
			
			// Only generate this code if there are actually actions to perform.
			if (!scopeHasExitAction(function, isExceptionState)) return;
			
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
				
				performScopeExitAction(function, pos, isExceptionState);
			}
			
			// ...and another to make it clear where it ends.
			const auto scopeDestroyEndBB = function.createBasicBlock(makeString("scopeExitActions_%llu_END", (unsigned long long) scopeLevel));
			function.getBuilder().CreateBr(scopeDestroyEndBB);
			function.selectBasicBlock(scopeDestroyEndBB);
		}
		
		void genAllScopeExitActions(Function& function, bool isExceptionState) {
			const auto& unwindStack = function.unwindStack();
			
			// Create a new basic block to make this clearer.
			const auto scopeDestroyAllBB = function.createBasicBlock("exitAllScopes");
			function.getBuilder().CreateBr(scopeDestroyAllBB);
			function.selectBasicBlock(scopeDestroyAllBB);
			
			for (size_t i = 0; i < unwindStack.size(); i++) {
				// Perform actions in reverse order (i.e. as a stack).
				const size_t pos = unwindStack.size() - i - 1;
				performScopeExitAction(function, pos, isExceptionState);
			}
		}
		
		LifetimeScope::LifetimeScope(Function& function)
			: function_(function) {
			const size_t scopeLevel = function_.unwindStack().size();
			const auto scopeStartBB = function_.createBasicBlock(makeString("scope_%llu_START", (unsigned long long) scopeLevel));
			function_.getBuilder().CreateBr(scopeStartBB);
			function_.selectBasicBlock(scopeStartBB);
			
			function_.unwindStack().push_back(UnwindAction::ScopeMarker());
		}
		
		LifetimeScope::~LifetimeScope() {
			const bool isExceptionState = false;
			genScopeExitActions(function_, isExceptionState);
			popScope(function_.unwindStack());
			
			const size_t scopeLevel = function_.unwindStack().size();
			const auto scopeEndBB = function_.createBasicBlock(makeString("scope_%llu_END", (unsigned long long) scopeLevel));
			function_.getBuilder().CreateBr(scopeEndBB);
			function_.selectBasicBlock(scopeEndBB);
		}
		
	}
	
}

