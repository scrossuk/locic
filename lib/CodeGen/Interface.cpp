#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Value* makeTypeInfoValue(Function& function, llvm::Value* vtablePointer, llvm::Value* templateGenerator) {
			llvm::Value* typeInfoValue = ConstantGenerator(function.module()).getUndef(typeInfoType(function.module()).second);
			typeInfoValue = function.getBuilder().CreateInsertValue(typeInfoValue, vtablePointer, { 0 });
			typeInfoValue = function.getBuilder().CreateInsertValue(typeInfoValue, templateGenerator, { 1 });
			return typeInfoValue;
		}
		
		llvm::Type* interfaceStructLLVMType(Module& module) {
			TypeGenerator typeGen(module);
			return typeGen.getStructType({ typeGen.getI8PtrType(), typeInfoType(module).second });
		}
		
		llvm_abi::Type interfaceStructABIType(Module& module) {
			std::vector<llvm_abi::Type> types;
			types.push_back(llvm_abi::Type::Pointer());
			types.push_back(std::move(typeInfoType(module).first));
			return llvm_abi::Type::AutoStruct(std::move(types));
		}
		
		TypePair interfaceStructType(Module& module) {
			return std::make_pair(interfaceStructABIType(module), interfaceStructLLVMType(module));
		}
		
		llvm::Value* makeInterfaceStructValue(Function& function, llvm::Value* contextPointer, llvm::Value* typeInfoValue) {
			llvm::Value* interfaceValue = ConstantGenerator(function.module()).getUndef(interfaceStructLLVMType(function.module()));
			interfaceValue = function.getBuilder().CreateInsertValue(interfaceValue, contextPointer, { 0 });
			interfaceValue = function.getBuilder().CreateInsertValue(interfaceValue, typeInfoValue, { 1 });
			return interfaceValue;
		}
		
		llvm::Type* interfaceMethodLLVMType(Module& module) {
			TypeGenerator typeGen(module);
			return typeGen.getStructType({ interfaceStructLLVMType(module), typeGen.getI64Type() });
		}
		
		llvm_abi::Type interfaceMethodABIType(Module& module) {
			std::vector<llvm_abi::Type> types;
			types.push_back(interfaceStructABIType(module));
			types.push_back(llvm_abi::Type::Integer(llvm_abi::Int64));
			return llvm_abi::Type::AutoStruct(std::move(types));
		}
		
		TypePair interfaceMethodType(Module& module) {
			return std::make_pair(interfaceMethodABIType(module), interfaceMethodLLVMType(module));
		}
		
		llvm::Value* makeInterfaceMethodValue(Function& function, llvm::Value* interfaceStructValue, llvm::Value* hashValue) {
			llvm::Value* methodValue = ConstantGenerator(function.module()).getUndef(interfaceMethodLLVMType(function.module()));
			methodValue = function.getBuilder().CreateInsertValue(methodValue, interfaceStructValue, { 0 });
			methodValue = function.getBuilder().CreateInsertValue(methodValue, hashValue, { 1 });
			return methodValue;
		}
		
	}
	
}

