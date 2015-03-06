#ifndef LOCIC_CODEGEN_ARGPAIR_HPP
#define LOCIC_CODEGEN_ARGPAIR_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>

namespace locic {
	
	namespace CodeGen {
		
		class ArgPair {
		public:
			ArgPair(llvm::Value* const argLLVMValue, const bool argHasPendingBind)
			: llvmValue_(argLLVMValue), hasPendingBind_(argHasPendingBind) { }
			
			llvm::Value* llvmValue() const {
				return llvmValue_;
			}
			
			bool hasPendingBind() const {
				return hasPendingBind_;
			}
			
		private:
			llvm::Value* llvmValue_;
			bool hasPendingBind_;
			
		};
		
	}
	
}

#endif
