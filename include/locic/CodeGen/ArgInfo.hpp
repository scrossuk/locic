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
				static ArgInfo None();
				
				static ArgInfo ContextOnly();
				
				static ArgInfo TemplateOnly();
				
				static ArgInfo TemplateAndContext();
				
				static ArgInfo Basic(std::vector<llvm_abi::Type> standardArguments, const std::vector<llvm::Type*>& argTypes);
				
				ArgInfo(bool hRVA, bool hTG, bool hCA, std::vector<llvm_abi::Type> standardArguments, const std::vector<llvm::Type*>& argTypes);
				
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
		
		ArgInfo getFunctionArgInfo(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* function);
		
		ArgInfo getTemplateVarFunctionStubArgInfo(Module& module, SEM::Function* function);
		
	}
	
}

#endif
