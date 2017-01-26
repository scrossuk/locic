#ifndef LOCIC_CODEGEN_ARGINFO_HPP
#define LOCIC_CODEGEN_ARGINFO_HPP

#include <cstdint>
#include <string>
#include <vector>

#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Type.hpp>

namespace locic {
	
	namespace AST {
		
		class FunctionType;
		class Type;
		
	}
	
	namespace CodeGen {
		
		class Module;
		
		struct ArgOffsets {
			size_t nestArgumentOffset;
			size_t returnVarArgumentOffset;
			size_t templateGeneratorArgumentOffset;
			size_t contextArgumentOffset;
			size_t standardArgumentOffset;
			size_t numArguments;
			
			ArgOffsets() :
			nestArgumentOffset(-1),
			returnVarArgumentOffset(-1),
			templateGeneratorArgumentOffset(-1),
			contextArgumentOffset(-1),
			standardArgumentOffset(0),
			numArguments(0) { }
		};
		
		class ArgInfo {
			public:
				static ArgInfo FromAST(Module& module, AST::FunctionType functionType);
				
				static ArgInfo VoidNone(Module& module);
				
				static ArgInfo VoidContextOnly(Module& module);
				
				static ArgInfo VoidContextWithArgs(Module& module, llvm::ArrayRef<llvm_abi::Type> argumentTypes);
				
				static ArgInfo VoidTemplateOnly(Module& module);
				
				static ArgInfo ContextOnly(Module& module, llvm_abi::Type returnType);
				
				static ArgInfo Templated(Module& module, llvm_abi::Type returnType, llvm::ArrayRef<llvm_abi::Type> argumentTypes);
				
				static ArgInfo TemplateOnly(Module& module, llvm_abi::Type returnType);
				
				static ArgInfo VoidTemplateAndContext(Module& module);
				
				static ArgInfo VoidTemplateAndContextWithArgs(Module& module, llvm::ArrayRef<llvm_abi::Type> argumentTypes);
				
				static ArgInfo TemplateAndContext(Module& module, llvm_abi::Type returnType);
				
				static ArgInfo VoidBasic(Module& module, llvm::ArrayRef<llvm_abi::Type> argumentTypes);
				
				static ArgInfo Basic(Module& module, llvm_abi::Type returnType, llvm::ArrayRef<llvm_abi::Type> argumentTypes);
				
				static ArgInfo VoidVarArgs(Module& module, llvm::ArrayRef<llvm_abi::Type> argumentTypes);
				
				static ArgInfo VarArgs(Module& module, llvm_abi::Type returnType, llvm::ArrayRef<llvm_abi::Type> argumentTypes);
				
				ArgInfo(Module& module, bool hRVA, bool hTG, bool hCA, bool pIsVarArg, llvm_abi::Type returnType,
				        llvm::ArrayRef<llvm_abi::Type> argumentTypes);
				
				ArgInfo withNoMemoryAccess() const;
				
				ArgInfo withNoExcept() const;
				
				ArgInfo withNoReturn() const;
				
				ArgInfo withNestArgument() const;
				
				llvm_abi::FunctionType getABIFunctionType() const;
				
				llvm::FunctionType* makeFunctionType() const;
				
				llvm::Function*
				createFunction(const llvm::Twine& name,
				               llvm::GlobalValue::LinkageTypes linkage) const;
				
				bool hasReturnVarArgument() const;
				
				bool hasTemplateGeneratorArgument() const;
				
				bool hasContextArgument() const;
				
				bool hasNestArgument() const;
				
				bool isVarArg() const;
				
				bool noMemoryAccess() const;
				
				bool noExcept() const;
				
				bool noReturn() const;
				
				ArgOffsets argumentOffsets() const;
				
				size_t numStandardArguments() const;
				
				size_t nestArgumentOffset() const;
				
				size_t returnVarArgumentOffset() const;
				
				size_t templateGeneratorArgumentOffset() const;
				
				size_t contextArgumentOffset() const;
				
				size_t standardArgumentOffset() const;
				
				size_t numArguments() const;
				
				const llvm_abi::Type& returnType() const;
				
				const llvm::SmallVector<llvm_abi::Type, 10>& argumentTypes() const;
				
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
				llvm_abi::Type returnType_;
				llvm::SmallVector<llvm_abi::Type, 10> argumentTypes_;
				
		};
		
	}
	
}

#endif
