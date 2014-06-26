#include <locic/SEM.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		UnwindAction::UnwindAction(Kind pKind, llvm::BasicBlock* pNormalUnwindBlock, llvm::BasicBlock* pExceptUnwindBlock)
			: kind_(pKind), normalUnwindBlock_(pNormalUnwindBlock),
			exceptUnwindBlock_(pExceptUnwindBlock) { }
		
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
		
		bool UnwindAction::isDestroyException() const {
			return kind() == UnwindAction::DESTROYEXCEPTION;
		}
		
		llvm::BasicBlock* UnwindAction::normalUnwindBlock() const {
			return normalUnwindBlock_;
		}
		
		llvm::BasicBlock* UnwindAction::exceptUnwindBlock() const {
			return exceptUnwindBlock_;
		}
		
	}
	
}

