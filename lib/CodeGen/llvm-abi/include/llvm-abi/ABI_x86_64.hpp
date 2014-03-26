#ifndef LLVMABI_ABI_X86_64_HPP
#define LLVMABI_ABI_X86_64_HPP

#include <vector>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Type.hpp>

namespace llvm_abi {

	class ABI_x86_64: public ABI {
		public:
			ABI_x86_64(llvm::LLVMContext& llvmContext);
			~ABI_x86_64();
			
			std::string name() const;
			
			const llvm::DataLayout& dataLayout() const;
			
			size_t typeSize(const Type& type);
			
			size_t typeAlign(const Type& type);
			
			llvm::Type* longDoubleType() const;
			
			std::vector<llvm::Value*> encodeValues(llvm::IRBuilder<>& entryBuilder, llvm::IRBuilder<>& builder, const std::vector<llvm::Value*>& argValues, const std::vector<Type>& argTypes);
			
			std::vector<llvm::Value*> decodeValues(llvm::IRBuilder<>& entryBuilder, llvm::IRBuilder<>& builder, const std::vector<llvm::Value*>& argValues, const std::vector<Type>& argTypes, const std::vector<llvm::Type*>& llvmArgTypes);
			
			llvm::FunctionType* rewriteFunctionType(llvm::FunctionType* llvmFunctionType, const FunctionType& functionType);
			
		private:
			llvm::LLVMContext& llvmContext_;
			llvm::DataLayout dataLayout_;
		
	};

}

#endif
