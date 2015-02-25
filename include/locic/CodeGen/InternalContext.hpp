#ifndef LOCIC_CODEGEN_INTERNALCONTEXT_HPP
#define LOCIC_CODEGEN_INTERNALCONTEXT_HPP

#include <llvm-abi/Context.hpp>

namespace locic {
	
	namespace CodeGen {
		
		class InternalContext {
			public:
				InternalContext();
				~InternalContext();
				
				llvm::LLVMContext& llvmContext();
				
				llvm_abi::Context& llvmABIContext();
				
			private:
				llvm::LLVMContext llvmContext_;
				llvm_abi::Context llvmABIContext_;
				
		};
		
	}
	
}

#endif
