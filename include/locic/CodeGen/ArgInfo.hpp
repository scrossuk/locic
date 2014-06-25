#ifndef LOCIC_CODEGEN_ARGINFO_HPP
#define LOCIC_CODEGEN_ARGINFO_HPP

#include <stdint.h>

#include <vector>

#include <llvm-abi/Type.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		TypePair voidTypePair(Module& module);
		
		TypePair sizeTypePair(Module& module);
		
		TypePair pointerTypePair(Module& module);
		
		class ArgInfo {
			public:
				static ArgInfo VoidNone(Module& module);
				
				static ArgInfo VoidContextOnly(Module& module);
				
				static ArgInfo VoidTemplateOnly(Module& module);
				
				static ArgInfo Templated(Module& module, TypePair returnType, const std::vector<TypePair>& argumentTypes);
				
				static ArgInfo TemplateOnly(Module& module, TypePair returnType);
				
				static ArgInfo VoidTemplateAndContext(Module& module);
				
				static ArgInfo TemplateAndContext(Module& module, TypePair returnType);
				
				static ArgInfo Basic(Module& module, TypePair returnType, const std::vector<TypePair>& argumentTypes);
				
				ArgInfo(Module& module, bool hRVA, bool hTG, bool hCA, bool pIsVarArg, TypePair returnType, const std::vector<TypePair>& argumentTypes);
				
				ArgInfo(ArgInfo&&) = default;
				ArgInfo& operator=(ArgInfo&&) = default;
				
				llvm::FunctionType* makeFunctionType() const;
				
				bool hasReturnVarArgument() const;
				
				bool hasTemplateGeneratorArgument() const;
				
				bool hasContextArgument() const;
				
				bool isVarArg() const;
				
				size_t returnVarArgumentOffset() const;
				
				size_t templateGeneratorArgumentOffset() const;
				
				size_t contextArgumentOffset() const;
				
				size_t standardArgumentOffset() const;
				
				size_t numStandardArguments() const;
				
				size_t numArguments() const;
				
				const TypePair& returnType() const;
				
				const std::vector<TypePair>& argumentTypes() const;
				
			private:
				// Non-copyable.
				ArgInfo(const ArgInfo&) = delete;
				ArgInfo& operator=(const ArgInfo&) = delete;
				
				Module& module_;
				bool hasReturnVarArgument_;
				bool hasTemplateGeneratorArgument_;
				bool hasContextArgument_;
				bool isVarArg_;
				size_t numStandardArguments_;
				
				TypePair returnType_;
				std::vector<TypePair> argumentTypes_;
				
		};
		
		ArgInfo getFunctionArgInfo(Module& module, SEM::Type* functionType);
		
		ArgInfo getTemplateVarFunctionStubArgInfo(Module& module, SEM::Function* function);
		
	}
	
}

#endif
