#ifndef LOCIC_CODEGEN_INTERNALCONTEXT_HPP
#define LOCIC_CODEGEN_INTERNALCONTEXT_HPP

#include <memory>

namespace locic {
	
	class MethodID;
	class PrimitiveID;
	class SharedMaps;
	class String;
	class StringHost;
	
	namespace SEM {
		
		class Context;
		
	}
	
	namespace CodeGen {
		
		struct TargetOptions;
		
		class InternalContext {
			public:
				InternalContext(const SEM::Context& semContext,
				                const SharedMaps& sharedMaps,
				                const TargetOptions& targetOptions);
				~InternalContext();
				
				const StringHost& stringHost() const;
				
				MethodID getMethodID(const String& name) const;
				
				PrimitiveID getPrimitiveID(const String& name) const;
				
				const SEM::Context& semContext() const;
				
				llvm::LLVMContext& llvmContext();
				
				const llvm::Triple& targetTriple() const;
				
				const llvm::Target* target() const;
				
				const llvm::TargetMachine& targetMachine() const;
				
				llvm::DataLayout dataLayout() const;
				
			private:
				const SEM::Context& semContext_;
				const SharedMaps& sharedMaps_;
				llvm::LLVMContext llvmContext_;
				llvm::Triple targetTriple_;
				const llvm::Target* target_;
				std::unique_ptr<llvm::TargetMachine> targetMachine_;
				
		};
		
	}
	
}

#endif
