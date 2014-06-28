#ifndef LLVMABI_ABI_X86_64_HPP
#define LLVMABI_ABI_X86_64_HPP

#include <unordered_map>
#include <vector>

#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Type.hpp>

namespace llvm_abi {

	using ABISizeCache = std::unordered_map<llvm_abi::Type*, size_t>;
	using ABITypeCache = std::unordered_map<llvm_abi::Type*, llvm::Type*>;
	
	class ABI_x86_64: public ABI {
		public:
			ABI_x86_64(llvm::Module* module);
			~ABI_x86_64();
			
			std::string name() const;
			
			const llvm::DataLayout& dataLayout() const;
			
			size_t typeSize(Type* type) const;
			
			size_t typeAlign(Type* type) const;
			
			llvm::Type* abiType(llvm_abi::Type* type) const;
			
			std::vector<size_t> calculateStructOffsets(llvm::ArrayRef<StructMember> structMembers) const;
			
			llvm::Type* longDoubleType() const;
			
			void encodeValues(IRBuilder& entryBuilder, IRBuilder& builder, std::vector<llvm::Value*>& argValues, llvm::ArrayRef<Type*> argTypes);
			
			void decodeValues(IRBuilder& entryBuilder, IRBuilder& builder, std::vector<llvm::Value*>& argValues, llvm::ArrayRef<Type*> argTypes, llvm::ArrayRef<llvm::Type*> llvmArgTypes);
			
			llvm::FunctionType* rewriteFunctionType(llvm::FunctionType* llvmFunctionType, const FunctionType& functionType);
			
		private:
			llvm::LLVMContext& llvmContext_;
			mutable ABITypeCache abiTypeCache_;
			mutable ABISizeCache alignOfCache_;
			mutable ABISizeCache sizeOfCache_;
			llvm::DataLayout dataLayout_;
			llvm::Value* memcpyIntrinsic_;
		
	};

}

#endif
