#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Value* makeTypeInfoValue(Function& function, llvm::Value* vtablePointer, llvm::Value* templateGenerator) {
			llvm::Value* typeInfoValue = ConstantGenerator(function.module()).getUndef(typeInfoType(function.module()));
			typeInfoValue = function.getBuilder().CreateInsertValue(typeInfoValue, vtablePointer, { 0 });
			typeInfoValue = function.getBuilder().CreateInsertValue(typeInfoValue, templateGenerator, { 1 });
			return typeInfoValue;
		}
		
		llvm::Type* interfaceStructType(Module& module) {
			TypeGenerator typeGen(module);
			return typeGen.getStructType({ typeGen.getI8PtrType(), typeInfoType(module) });
		}
		
		llvm_abi::Type interfaceStructABIType() {
			std::vector<llvm_abi::Type> types;
			types.push_back(llvm_abi::Type::Pointer());
			types.push_back(typeInfoABIType());
			return llvm_abi::Type::AutoStruct(std::move(types));
		}
		
		llvm::Value* makeInterfaceStructValue(Function& function, llvm::Value* contextPointer, llvm::Value* typeInfoValue) {
			llvm::Value* interfaceValue = ConstantGenerator(function.module()).getUndef(interfaceStructType(function.module()));
			interfaceValue = function.getBuilder().CreateInsertValue(interfaceValue, contextPointer, { 0 });
			interfaceValue = function.getBuilder().CreateInsertValue(interfaceValue, typeInfoValue, { 1 });
			return interfaceValue;
		}
		
		llvm::Type* interfaceMethodType(Module& module) {
			TypeGenerator typeGen(module);
			return typeGen.getStructType({ interfaceStructType(module), typeGen.getI64Type() });
		}
		
		llvm_abi::Type interfaceMethodABIType() {
			std::vector<llvm_abi::Type> types;
			types.push_back(interfaceStructABIType());
			types.push_back(llvm_abi::Type::Integer(llvm_abi::Int64));
			return llvm_abi::Type::AutoStruct(std::move(types));
		}
		
		llvm::Value* makeInterfaceMethodValue(Function& function, llvm::Value* interfaceStructValue, llvm::Value* hashValue) {
			llvm::Value* methodValue = ConstantGenerator(function.module()).getUndef(interfaceMethodType(function.module()));
			methodValue = function.getBuilder().CreateInsertValue(methodValue, interfaceStructValue, { 0 });
			methodValue = function.getBuilder().CreateInsertValue(methodValue, hashValue, { 1 });
			return methodValue;
		}
		
	}
	
}

