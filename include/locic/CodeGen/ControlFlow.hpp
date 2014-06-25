#ifndef LOCIC_CODEGEN_CONTROLFLOW_HPP
#define LOCIC_CODEGEN_CONTROLFLOW_HPP

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		void genControlFlowBreak(Function& function);
		
		void genControlFlowContinue(Function& function);
		
		class ControlFlowScope {
			public:
				ControlFlowScope(Function& function, llvm::BasicBlock* breakBB, llvm::BasicBlock* continueBB);
				~ControlFlowScope();
				
			private:
				UnwindStack& unwindStack_;
			
		};
		
	}
	
}

#endif
