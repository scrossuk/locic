#ifndef LOCIC_CODEGEN_UNWINDACTION_HPP
#define LOCIC_CODEGEN_UNWINDACTION_HPP

#include <array>
#include <bitset>
#include <vector>

#include <locic/SEM.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/UnwindState.hpp>

namespace locic {

	namespace CodeGen {
	
		enum ScopeExitState {
			SCOPEEXIT_ALWAYS,
			SCOPEEXIT_SUCCESS,
			SCOPEEXIT_FAILURE
		};
		
		class UnwindAction {
			public:
				static UnwindAction Destructor(SEM::Type* type, llvm::Value* value);
				
				static UnwindAction CatchException(llvm::BasicBlock* catchBlock, llvm::Constant* catchTypeInfo);
				
				static UnwindAction ScopeMarker(llvm::BasicBlock* scopeEndBlock);
				
				static UnwindAction FunctionMarker();
				
				static UnwindAction ControlFlow(llvm::BasicBlock* breakBlock, llvm::BasicBlock* continueBlock);
				
				static UnwindAction ScopeExit(ScopeExitState state, SEM::Scope* scope);
				
				static UnwindAction DestroyException(llvm::Value* exceptionValue);
				
				enum Kind {
					DESTRUCTOR,
					CATCH,
					SCOPEMARKER,
					FUNCTIONMARKER,
					CONTROLFLOW,
					SCOPEEXIT,
					DESTROYEXCEPTION
				};
				
				Kind kind() const;
				
				bool isDestructor() const;
				
				bool isCatch() const;
				
				bool isScopeMarker() const;
				
				bool isFunctionMarker() const;
				
				bool isControlFlow() const;
				
				bool isScopeExit() const;
				
				bool isDestroyException() const;
				
				SEM::Type* destructorType() const;
				
				llvm::Value* destructorValue() const;
				
				llvm::BasicBlock* catchBlock() const;
				
				llvm::Constant* catchTypeInfo() const;
				
				llvm::BasicBlock* scopeEndBlock() const;
				
				llvm::BasicBlock* breakBlock() const;
				
				llvm::BasicBlock* continueBlock() const;
				
				ScopeExitState scopeExitState() const;
				
				SEM::Scope* scopeExitScope() const;
				
				llvm::Value* destroyExceptionValue() const;
				
				bool isTerminator() const;
				
				bool isActiveForState(UnwindState unwindState) const;
				
				llvm::BasicBlock* actionBlock(UnwindState state);
				
				void setActionBlock(UnwindState state, llvm::BasicBlock* actionBB);
				
				bool hasSuccessor(llvm::BasicBlock* successorBB) const;
				
				void addSuccessor(llvm::BasicBlock* successorBB);
				
				llvm::BasicBlock* landingPadBlock() const;
				
				void setLandingPadBlock(llvm::BasicBlock* landingPadBB);
				
			private:
				inline UnwindAction(Kind pKind)
					: kind_(pKind),
					landingPadBB_(nullptr) {
						actionBB_.fill(nullptr);
						successorBB_.fill(nullptr);
					}
					
				Kind kind_;
				
				llvm::BasicBlock* landingPadBB_;
				
				std::array<llvm::BasicBlock*, UnwindState_MAX> actionBB_;
				std::array<llvm::BasicBlock*, UnwindState_MAX> successorBB_;
				
				union Actions {
					struct DestructorAction {
						SEM::Type* type;
						llvm::Value* value;
					} destructorAction;
					
					struct CatchAction {
						llvm::BasicBlock* block;
						llvm::Constant* typeInfo;
					} catchAction;
					
					struct ScopeAction {
						llvm::BasicBlock* block;
					} scopeAction;
					
					struct ControlFlowAction {
						llvm::BasicBlock* breakBlock;
						llvm::BasicBlock* continueBlock;
					} controlFlowAction;
					
					struct ScopeExitAction {
						ScopeExitState state;
						SEM::Scope* scope;
					} scopeExitAction;
					
					struct DestroyExceptionAction {
						llvm::Value* exceptionValue;
					} destroyExceptionAction;
				} actions_;
				
		};
		
	}
	
}

#endif
