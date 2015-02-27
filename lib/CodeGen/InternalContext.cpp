#include <llvm-abi/Context.hpp>

#include <locic/CodeGen/InternalContext.hpp>

namespace locic {

	namespace CodeGen {
		
		InternalContext::InternalContext(const StringHost& argStringHost) : stringHost_(argStringHost) { }
		
		InternalContext::~InternalContext() { }
		
		const StringHost& InternalContext::stringHost() const {
			return stringHost_;
		}
		
		llvm::LLVMContext& InternalContext::llvmContext() {
			return llvmContext_;
		}
		
		llvm_abi::Context& InternalContext::llvmABIContext() {
			return llvmABIContext_;
		}
		
	}
	
}

