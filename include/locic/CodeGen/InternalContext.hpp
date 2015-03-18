#ifndef LOCIC_CODEGEN_INTERNALCONTEXT_HPP
#define LOCIC_CODEGEN_INTERNALCONTEXT_HPP

#include <memory>

#include <llvm-abi/Context.hpp>

namespace locic {
	
	class StringHost;
	
	namespace CodeGen {
		
		struct TargetOptions;
		
		class InternalContext {
			public:
				InternalContext(const StringHost& stringHost, const TargetOptions& targetOptions);
				~InternalContext();
				
				const StringHost& stringHost() const;
				
				llvm::LLVMContext& llvmContext();
				
				llvm_abi::Context& llvmABIContext();
				
				const llvm::Triple& targetTriple() const;
				
				const llvm::Target* target() const;
				
				const llvm::TargetMachine& targetMachine() const;
				
				const llvm::DataLayout& dataLayout() const;
				
			private:
				const StringHost& stringHost_;
				llvm::LLVMContext llvmContext_;
				llvm_abi::Context llvmABIContext_;
				llvm::Triple targetTriple_;
				const llvm::Target* target_;
				std::unique_ptr<llvm::TargetMachine> targetMachine_;
				
		};
		
	}
	
}

#endif
