#ifndef LOCIC_CODEGEN_UNWINDACTION_HPP
#define LOCIC_CODEGEN_UNWINDACTION_HPP

#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/SEM.hpp>

namespace locic {

	namespace CodeGen {
	
		class UnwindAction {
			public:
				enum Kind {
					DESTRUCTOR,
					CATCH,
					SCOPEMARKER,
					STATEMENTMARKER,
					CONTROLFLOW,
					SCOPEEXIT,
					DESTROYEXCEPTION
				};
				
				UnwindAction(Kind pKind, llvm::BasicBlock* pNormalUnwindBlock, llvm::BasicBlock* pExceptUnwindBlock);
				
				Kind kind() const;
				
				bool isDestructor() const;
				
				bool isCatch() const;
				
				bool isScopeMarker() const;
				
				bool isStatementMarker() const;
				
				bool isControlFlow() const;
				
				bool isScopeExit() const;
				
				bool isDestroyException() const;
				
				llvm::BasicBlock* normalUnwindBlock() const;
				
				llvm::BasicBlock* exceptUnwindBlock() const;
				
			private:
				inline UnwindAction(Kind pKind)
					: kind_(pKind) { }
				
				Kind kind_;
				llvm::BasicBlock* normalUnwindBlock_;
				llvm::BasicBlock* exceptUnwindBlock_;
				
		};
		
	}
	
}

#endif
