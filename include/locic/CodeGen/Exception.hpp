#ifndef LOCIC_CODEGEN_EXCEPTION_HPP
#define LOCIC_CODEGEN_EXCEPTION_HPP

#include <vector>

#include <locic/SEM.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Function* getExceptionAllocateFunction(Module& module);
		
		llvm::Function* getExceptionFreeFunction(Module& module);
		
		llvm::Function* getExceptionThrowFunction(Module& module);
		
		llvm::Function* getExceptionRethrowFunction(Module& module);
		
		llvm::Function* getExceptionPersonalityFunction(Module& module);
		
		llvm::Function* getExceptionPtrFunction(Module& module);
		
		void genLandingPad(Function& function, bool isRethrow);
		
		void scheduleExceptionDestroy(Function& function, llvm::Value* exceptionPtrValue);
		
		llvm::Constant* genCatchInfo(Module& module, SEM::TypeInstance* catchTypeInstance);
		
		llvm::Constant* genThrowInfo(Module& module, SEM::TypeInstance* throwTypeInstance);
		
		class TryScope {
			public:
				TryScope(Function& function, llvm::BasicBlock* catchBlock, const std::vector<llvm::Constant*>& catchTypeList);
				~TryScope();
				
			private:
				// Non-copyable.
				TryScope(const TryScope&) = delete;
				TryScope& operator=(TryScope) = delete;
				
				Function& function_;
				size_t catchCount_;
			
		};
		
	}
	
}

#endif
