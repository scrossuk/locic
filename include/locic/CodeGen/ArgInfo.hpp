#ifndef LOCIC_CODEGEN_ARGINFO_HPP
#define LOCIC_CODEGEN_ARGINFO_HPP

#include <stdint.h>

#include <vector>

#include <llvm-abi/Type.hpp>

#include <locic/SEM.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		class ArgInfo {
			public:
				static ArgInfo None(Module& module);
				
				static ArgInfo ContextOnly(Module& module);
				
				static ArgInfo TemplateOnly(Module& module);
				
				static ArgInfo TemplateAndContext(Module& module);
				
				static ArgInfo Basic(Module& module, std::vector<llvm_abi::Type> standardArguments, const std::vector<llvm::Type*>& argTypes);
				
				ArgInfo(Module& module, bool hRVA, bool hTG, bool hCA, std::vector<llvm_abi::Type> standardArguments, const std::vector<llvm::Type*>& argTypes);
				
				ArgInfo(ArgInfo&&) = default;
				ArgInfo& operator=(ArgInfo&&) = default;
					  
				bool hasReturnVarArgument() const;
				
				bool hasTemplateGeneratorArgument() const;
				
				bool hasContextArgument() const;
				
				size_t returnVarArgumentOffset() const;
				
				size_t templateGeneratorArgumentOffset() const;
				
				size_t contextArgumentOffset() const;
				
				size_t standardArgumentOffset() const;
				
				size_t numStandardArguments() const;
				
				size_t numArguments() const;
				
				const std::vector<llvm_abi::Type>& abiTypes() const;
				
				const std::vector<llvm::Type*>& abiLLVMTypes() const;
				
			private:
				// Non-copyable.
				ArgInfo(const ArgInfo&) = delete;
				ArgInfo& operator=(const ArgInfo&) = delete;
				
				bool hasReturnVarArgument_;
				bool hasTemplateGeneratorArgument_;
				bool hasContextArgument_;
				size_t numStandardArguments_;
				std::vector<llvm_abi::Type> abiTypes_;
				std::vector<llvm::Type*> abiLLVMTypes_;
				
		};
		
		ArgInfo getFunctionArgInfo(Module& module, SEM::Type* functionType);
		
		ArgInfo getTemplateVarFunctionStubArgInfo(Module& module, SEM::Function* function);
		
	}
	
}

#endif
