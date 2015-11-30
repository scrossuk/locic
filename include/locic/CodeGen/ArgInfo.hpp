#ifndef LOCIC_CODEGEN_ARGINFO_HPP
#define LOCIC_CODEGEN_ARGINFO_HPP

#include <cstdint>
#include <string>
#include <vector>

#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Type.hpp>

namespace locic {
	
	namespace SEM {
		
		class Function;
		class FunctionType;
		class Type;
		
	}
	
	namespace CodeGen {
		
		class Module;
		typedef std::pair<llvm_abi::Type, llvm::Type*> TypePair;
		
		TypePair voidTypePair(Module& module);
		
		TypePair boolTypePair(Module& module);
		
		TypePair sizeTypePair(Module& module);
		
		TypePair pointerTypePair(Module& module);
		
		class ArgInfo {
			public:
				static ArgInfo VoidNone(Module& module);
				
				static ArgInfo VoidContextOnly(Module& module);
				
				static ArgInfo VoidContextWithArgs(Module& module, llvm::ArrayRef<TypePair> argumentTypes);
				
				static ArgInfo VoidTemplateOnly(Module& module);
				
				static ArgInfo ContextOnly(Module& module, TypePair returnType);
				
				static ArgInfo Templated(Module& module, TypePair returnType, llvm::ArrayRef<TypePair> argumentTypes);
				
				static ArgInfo TemplateOnly(Module& module, TypePair returnType);
				
				static ArgInfo VoidTemplateAndContext(Module& module);
				
				static ArgInfo VoidTemplateAndContextWithArgs(Module& module, llvm::ArrayRef<TypePair> argumentTypes);
				
				static ArgInfo TemplateAndContext(Module& module, TypePair returnType);
				
				static ArgInfo VoidBasic(Module& module, llvm::ArrayRef<TypePair> argumentTypes);
				
				static ArgInfo Basic(Module& module, TypePair returnType, llvm::ArrayRef<TypePair> argumentTypes);
				
				static ArgInfo VarArgs(Module& module, TypePair returnType, llvm::ArrayRef<TypePair> argumentTypes);
				
				ArgInfo(Module& module, bool hRVA, bool hTG, bool hCA, bool pIsVarArg, TypePair returnType, llvm::ArrayRef<TypePair> argumentTypes);
				
				ArgInfo withNoMemoryAccess() const;
				
				ArgInfo withNoExcept() const;
				
				ArgInfo withNoReturn() const;
				
				ArgInfo withNestArgument() const;
				
				llvm_abi::FunctionType getABIFunctionType() const;
				
				llvm::FunctionType* makeFunctionType() const;
				
				bool hasReturnVarArgument() const;
				
				bool hasTemplateGeneratorArgument() const;
				
				bool hasContextArgument() const;
				
				bool hasNestArgument() const;
				
				bool isVarArg() const;
				
				bool noMemoryAccess() const;
				
				bool noExcept() const;
				
				bool noReturn() const;
				
				size_t nestArgumentOffset() const;
				
				size_t returnVarArgumentOffset() const;
				
				size_t templateGeneratorArgumentOffset() const;
				
				size_t contextArgumentOffset() const;
				
				size_t standardArgumentOffset() const;
				
				size_t numStandardArguments() const;
				
				size_t numArguments() const;
				
				const TypePair& returnType() const;
				
				const llvm::SmallVector<TypePair, 10>& argumentTypes() const;
				
				std::string toString() const;
				
			private:
				Module* module_;
				bool hasReturnVarArgument_;
				bool hasTemplateGeneratorArgument_;
				bool hasContextArgument_;
				bool hasNestArgument_;
				bool isVarArg_;
				bool noMemoryAccess_;
				bool noExcept_;
				bool noReturn_;
				size_t numStandardArguments_;
				TypePair returnType_;
				llvm::SmallVector<TypePair, 10> argumentTypes_;
				
		};
		
		bool canPassByValue(Module& module, const SEM::Type* type);
		
		ArgInfo getFunctionArgInfo(Module& module, SEM::FunctionType functionType);
		
	}
	
}

#endif
