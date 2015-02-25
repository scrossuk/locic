#include <llvm-abi/Context.hpp>

#include <locic/CodeGen/InternalContext.hpp>

namespace locic {

	namespace CodeGen {
		
		InternalContext::InternalContext() { }
		
		InternalContext::~InternalContext() { }
		
		llvm::LLVMContext& InternalContext::llvmContext() {
			return llvmContext_;
		}
		
		llvm_abi::Context& InternalContext::llvmABIContext() {
			return llvmABIContext_;
		}
		
	}
	
}

