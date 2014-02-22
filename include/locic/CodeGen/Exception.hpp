#ifndef LOCIC_CODEGEN_EXCEPTION_HPP
#define LOCIC_CODEGEN_EXCEPTION_HPP

#include <locic/SEM.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Function* getExceptionAllocateFunction(Module& module);
		
		llvm::Function* getExceptionThrowFunction(Module& module);
		
		llvm::Function* getExceptionPersonalityFunction(Module& module);
		
		llvm::Function* getBeginCatchFunction(Module& module);
		
		llvm::Function* getEndCatchFunction(Module& module);
		
		void genLandingPad(Function& function);
		
		void genExceptionUnwind(Function& function);
		
		llvm::Constant* genCatchInfo(Module& module, SEM::TypeInstance* catchTypeInstance);
		
		llvm::Constant* genThrowInfo(Module& module, SEM::TypeInstance* throwTypeInstance);
		
		class TryScope {
			public:
				TryScope(UnwindStack& unwindStack, llvm::BasicBlock* catchBlock, llvm::Constant* catchTypeInfo);
				~TryScope();
				
			private:
				// Non-copyable.
				TryScope(const TryScope&) = delete;
				TryScope& operator=(TryScope) = delete;
				
				UnwindStack& unwindStack_;
			
		};
		
	}
	
}

#endif
