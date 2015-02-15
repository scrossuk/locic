#ifndef LOCIC_CODEGEN_SUPPORT_HPP
#define LOCIC_CODEGEN_SUPPORT_HPP

#include <locic/Array.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::StructType* vtableType(Module& module);
		
		template <typename T, size_t BaseSize>
		llvm::ArrayRef<T> arrayRef(const Array<T, BaseSize>& array) {
			return llvm::ArrayRef<T>(array.data(), array.size());
		}
		
	}
	
}

#endif
