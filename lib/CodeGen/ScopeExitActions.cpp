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
		
		llvm::BasicBlock* getNextNormalUnwindBlock(Function& function) {
			return function.unwindStack().back().normalUnwindBlock();
		}
		
		llvm::BasicBlock* getNextExceptUnwindBlock(Function& function) {
			return function.unwindStack().back().exceptUnwindBlock();
		}
		
		namespace {
			
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
		
		namespace {
			
			llvm::BasicBlock* genNormalOuterUnwindBlock(Function& function) {
				const auto currentBB = function.getSelectedBasicBlock();
				
				const auto unwindBB = function.createBasicBlock("");
				function.selectBasicBlock(unwindBB);
				
				// Return a value or void, depending on whether the
				// generated function returns a value directly.
				const auto rawReturnValue = function.getRawReturnValue();
				if (rawReturnValue != nullptr) {
					function.getBuilder().CreateRet(rawReturnValue);
				} else {
					function.getBuilder().CreateRetVoid();
				}
				
				function.selectBasicBlock(currentBB);
				
				return unwindBB;
			}
			
			llvm::BasicBlock* genExceptOuterUnwindBlock(Function& function) {
				const auto currentBB = function.getSelectedBasicBlock();
				
				const auto unwindBB = function.createBasicBlock("functionUnwind");
				function.selectBasicBlock(unwindBB);
				
				// Continue unwinding.
				const auto exceptionInfo = function.getBuilder().CreateLoad(function.exceptionInfo());
				function.getBuilder().CreateResume(exceptionInfo);
				
				function.selectBasicBlock(currentBB);
				
				return unwindBB;
			}
			
		}
		
		FunctionLifetime::FunctionLifetime(Function& function)
			: function_(function) {
				const auto normalOuterBB = genNormalOuterUnwindBlock(function);
				
				// TODO: only generate this if function isn't noexcept.
				const auto exceptOuterBB = genExceptOuterUnwindBlock(function);
				
				function_.unwindStack().push_back(UnwindAction(UnwindAction::SCOPEMARKER, normalOuterBB, exceptOuterBB));
			}
		
		FunctionLifetime::~FunctionLifetime() {
			// Jump to unwind block to perform exit actions, which should
			// then end up jumping to our unwind block.
			function_.getBuilder().CreateBr(getNextNormalUnwindBlock(function_));
		}
		
		ScopeLifetime::ScopeLifetime(Function& function)
			: function_(function),
			scopeExitBB_(function.createBasicBlock("scopeUnwind")) {
				function_.unwindStack().push_back(UnwindAction(UnwindAction::SCOPEMARKER, scopeExitBB_, getNextExceptUnwindBlock(function)));
			}
		
		ScopeLifetime::~ScopeLifetime() {
			// Jump to unwind block to perform exit actions, which should
			// then end up jumping to our unwind block.
			function_.getBuilder().CreateBr(getNextNormalUnwindBlock(function_));
			
			popScope(function_.unwindStack());
			
			// Generate our unwind block.
			function_.selectBasicBlock(scopeExitBB_);
			
			const auto stopUnwindingBB = function_.createBasicBlock("");
			
			// If this is normal execution, stop unwinding, otherwise
			// continue unwinding by jumping to the next unwind block.
			const auto isNormalExecution = getIsCurrentUnwindState(function_, UnwindStateNormal);
			
			function_.getBuilder().CreateCondBr(isNormalExecution, stopUnwindingBB, getNextNormalUnwindBlock(function_));
			
			function_.selectBasicBlock(stopUnwindingBB);
		}
		
		StatementLifetime::StatementLifetime(Function& function)
			: function_(function),
			statementExitBB_(function.createBasicBlock("statementUnwind")) {
				function_.unwindStack().push_back(UnwindAction(UnwindAction::STATEMENTMARKER, statementExitBB_, getNextExceptUnwindBlock(function)));
			}
		
		StatementLifetime::~StatementLifetime() {
			// Jump to unwind block to perform exit actions, which should
			// then end up jumping to our unwind block.
			function_.getBuilder().CreateBr(getNextNormalUnwindBlock(function_));
			
			popStatement(function_.unwindStack());
			
			// Generate our unwind block.
			function_.selectBasicBlock(statementExitBB_);
			
			const auto stopUnwindingBB = function_.createBasicBlock("");
			
			// If this is normal execution, stop unwinding, otherwise
			// continue unwinding by jumping to the next unwind block.
			const auto isNormalExecution = getIsCurrentUnwindState(function_, UnwindStateNormal);
			
			function_.getBuilder().CreateCondBr(isNormalExecution, stopUnwindingBB, getNextNormalUnwindBlock(function_));
			
			function_.selectBasicBlock(stopUnwindingBB);
		}
		
	}
	
}

