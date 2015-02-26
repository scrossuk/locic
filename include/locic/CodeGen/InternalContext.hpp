#ifndef LOCIC_CODEGEN_INTERNALCONTEXT_HPP
#define LOCIC_CODEGEN_INTERNALCONTEXT_HPP

#include <llvm-abi/Context.hpp>

namespace locic {
	
	class StringHost;
	
	namespace CodeGen {
		
		class InternalContext {
			public:
				InternalContext(const StringHost& stringHost);
				~InternalContext();
				
				const StringHost& stringHost() const;
				
				llvm::LLVMContext& llvmContext();
				
				llvm_abi::Context& llvmABIContext();
				
			private:
				const StringHost& stringHost_;
				llvm::LLVMContext llvmContext_;
				llvm_abi::Context llvmABIContext_;
				
		};
		
	}
	
}

#endif
