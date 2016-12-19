#ifndef LOCIC_CODEGEN_INTERNALCONTEXT_HPP
#define LOCIC_CODEGEN_INTERNALCONTEXT_HPP

#include <memory>

namespace locic {
	
	class MethodID;
	class PrimitiveID;
	class SharedMaps;
	class String;
	class StringHost;
	
	namespace AST {
		
		class Context;
		
	}
	
	namespace CodeGen {
		
		struct TargetOptions;
		
		class InternalContext {
			public:
				InternalContext(const AST::Context& semContext,
				                const SharedMaps& sharedMaps,
				                const TargetOptions& targetOptions);
				~InternalContext();
				
				const StringHost& stringHost() const;
				
				MethodID getMethodID(const String& name) const;
				
				PrimitiveID getPrimitiveID(const String& name) const;
				
				const AST::Context& semContext() const;
				
				llvm::LLVMContext& llvmContext();
				
				const llvm::Triple& targetTriple() const;
				
				const llvm::Target* target() const;
				
				const llvm::TargetMachine& targetMachine() const;
				
				llvm::DataLayout dataLayout() const;
				
			private:
				const AST::Context& semContext_;
				const SharedMaps& sharedMaps_;
				llvm::LLVMContext llvmContext_;
				llvm::Triple targetTriple_;
				const llvm::Target* target_;
				std::unique_ptr<llvm::TargetMachine> targetMachine_;
				
		};
		
	}
	
}

#endif
