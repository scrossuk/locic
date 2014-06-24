#include <memory>
#include <string>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABI_x86_64.hpp>

namespace llvm_abi {

	std::unique_ptr<ABI> createABI(llvm::Module* module, const std::string& targetTriple) {
		// TODO
		(void) targetTriple;
		return std::unique_ptr<ABI>(new ABI_x86_64(module));
	}
	
}

