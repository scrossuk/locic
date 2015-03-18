#include <memory>
#include <stdexcept>
#include <string>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABI_x86_64.hpp>

namespace llvm_abi {
	
	std::unique_ptr<ABI> createABI(llvm::Module* const module, const llvm::Triple& targetTriple) {
		switch (targetTriple.getArch()) {
			case llvm::Triple::x86_64: {
				if (targetTriple.isOSWindows()) {
					// Windows 64-bit target not yet supported.
					break;
				} else {
					return std::unique_ptr<ABI>(new ABI_x86_64(module));
				}
			}
			default:
				break;
		}
		
		std::string errorString = "No ABI available for triple: ";
		errorString += targetTriple.str();
		throw std::runtime_error(errorString);
	}
	
}

