#ifndef LOCIC_CODEGEN_UNWINDACTION_HPP
#define LOCIC_CODEGEN_UNWINDACTION_HPP

#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/SEM.hpp>

namespace locic {

	namespace CodeGen {
	
		enum ScopeExitState {
			SCOPEEXIT_ALWAYS,
			SCOPEEXIT_SUCCESS,
			SCOPEEXIT_FAILURE
		};
		
		class UnwindAction {
			public:
				static UnwindAction Destroy(llvm::BasicBlock* destroyBlock);
				
				static UnwindAction CatchException(llvm::BasicBlock* catchBlock, llvm::Constant* catchTypeInfo);
				
				static UnwindAction ScopeMarker(llvm::BasicBlock* scopeEndBlock);
				
				static UnwindAction StatementMarker(llvm::BasicBlock* statementEndBlock);
				
				static UnwindAction ControlFlow(llvm::BasicBlock* actionBlock);
				
				static UnwindAction ScopeExit(llvm::BasicBlock* actionBlock);
				
				static UnwindAction CatchBlock(llvm::BasicBlock* destroyBlock, llvm::Value* exceptionValue);
				
				enum Kind {
					DESTRUCTOR,
					CATCH,
					SCOPEMARKER,
					STATEMENTMARKER,
					CONTROLFLOW,
					SCOPEEXIT,
					CATCHBLOCK
				};
				
				Kind kind() const;
				
				bool isDestructor() const;
				
				bool isCatch() const;
				
				bool isScopeMarker() const;
				
				bool isStatementMarker() const;
				
				bool isControlFlow() const;
				
				bool isScopeExit() const;
				
				bool isCatchBlock() const;
				
				llvm::BasicBlock* destroyBlock() const;
				
				llvm::BasicBlock* catchBlock() const;
				
				llvm::Constant* catchTypeInfo() const;
				
				llvm::BasicBlock* scopeEndBlock() const;
				
				llvm::BasicBlock* statementEndBlock() const;
				
				llvm::BasicBlock* controlFlowBlock() const;
				
				llvm::BasicBlock* scopeExitBlock() const;
				
				llvm::BasicBlock* destroyExceptionBlock() const;
				
				llvm::Value* catchExceptionValue() const;
				
				llvm::BasicBlock* unwindBlock() const;
				
			private:
				inline UnwindAction(Kind pKind)
					: kind_(pKind) { }
				
				Kind kind_;
				
				union Actions {
					struct DestroyAction {
						llvm::BasicBlock* block;
						llvm::Value* value;
					} destroyAction;
					
					struct CatchAction {
						llvm::BasicBlock* block;
						llvm::Constant* typeInfo;
					} catchAction;
					
					struct ScopeMarker {
						llvm::BasicBlock* block;
					} scopeMarker;
					
					struct StatementMarker {
						llvm::BasicBlock* block;
					} statementMarker;
					
					struct ControlFlowAction {
						llvm::BasicBlock* block;
					} controlFlowAction;
					
					struct ScopeExitAction {
						llvm::BasicBlock* block;
					} scopeExitAction;
					
					struct CatchBlock {
						llvm::BasicBlock* block;
						llvm::Value* exceptionValue;
					} catchBlock;
				} actions_;
				
		};
		
		typedef std::vector<UnwindAction> UnwindStack;
		
	}
	
}

#endif
