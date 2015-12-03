#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Value* makeTypeInfoValue(Function& function, llvm::Value* vtablePointer, llvm::Value* templateGenerator) {
			IREmitter irEmitter(function);
			llvm::Value* typeInfoValue = ConstantGenerator(function.module()).getUndef(typeInfoType(function.module()).second);
			typeInfoValue = irEmitter.emitInsertValue(typeInfoValue, vtablePointer, { 0 });
			typeInfoValue = irEmitter.emitInsertValue(typeInfoValue, templateGenerator, { 1 });
			return typeInfoValue;
		}
		
		llvm_abi::Type interfaceStructABIType(Module& module) {
			llvm::SmallVector<llvm_abi::Type, 2> types;
			types.push_back(llvm_abi::PointerTy);
			types.push_back(typeInfoType(module).first);
			return module.abiTypeBuilder().getStructTy(types, "__interface_ref");
		}
		
		llvm::Type* interfaceStructLLVMType(Module& module) {
			return module.abi().typeInfo().getLLVMType(interfaceStructABIType(module));
		}
		
		TypePair interfaceStructType(Module& module) {
			return std::make_pair(interfaceStructABIType(module), interfaceStructLLVMType(module));
		}
		
		llvm::Value* makeInterfaceStructValue(Function& function, llvm::Value* contextPointer, llvm::Value* typeInfoValue) {
			IREmitter irEmitter(function);
			llvm::Value* interfaceValue = ConstantGenerator(function.module()).getUndef(interfaceStructLLVMType(function.module()));
			interfaceValue = irEmitter.emitInsertValue(interfaceValue, contextPointer, { 0 });
			interfaceValue = irEmitter.emitInsertValue(interfaceValue, typeInfoValue, { 1 });
			return interfaceValue;
		}
		
		llvm_abi::Type interfaceMethodABIType(Module& module) {
			llvm::SmallVector<llvm_abi::Type, 2> types;
			types.push_back(interfaceStructABIType(module));
			types.push_back(llvm_abi::Int64Ty);
			return module.abiTypeBuilder().getStructTy(types, "__interface_method");
		}
		
		llvm::Type* interfaceMethodLLVMType(Module& module) {
			return module.abi().typeInfo().getLLVMType(interfaceMethodABIType(module));
		}
		
		TypePair interfaceMethodType(Module& module) {
			return std::make_pair(interfaceMethodABIType(module), interfaceMethodLLVMType(module));
		}
		
		llvm::Value* makeInterfaceMethodValue(Function& function, llvm::Value* interfaceStructValue, llvm::Value* hashValue) {
			IREmitter irEmitter(function);
			llvm::Value* methodValue = ConstantGenerator(function.module()).getUndef(interfaceMethodLLVMType(function.module()));
			methodValue = irEmitter.emitInsertValue(methodValue, interfaceStructValue, { 0 });
			methodValue = irEmitter.emitInsertValue(methodValue, hashValue, { 1 });
			return methodValue;
		}
		
		llvm_abi::Type staticInterfaceMethodABIType(Module& module) {
			llvm::SmallVector<llvm_abi::Type, 2> types;
			types.push_back(typeInfoType(module).first);
			types.push_back(llvm_abi::Int64Ty);
			return module.abiTypeBuilder().getStructTy(types, "__static_interface_method");
		}
		
		llvm::Type* staticInterfaceMethodLLVMType(Module& module) {
			return module.abi().typeInfo().getLLVMType(staticInterfaceMethodABIType(module));
		}
		
		TypePair staticInterfaceMethodType(Module& module) {
			return std::make_pair(staticInterfaceMethodABIType(module), staticInterfaceMethodLLVMType(module));
		}
		
		llvm::Value* makeStaticInterfaceMethodValue(Function& function, llvm::Value* typeInfoValue, llvm::Value* hashValue) {
			IREmitter irEmitter(function);
			llvm::Value* methodValue = ConstantGenerator(function.module()).getUndef(staticInterfaceMethodLLVMType(function.module()));
			methodValue = irEmitter.emitInsertValue(methodValue, typeInfoValue, { 0 });
			methodValue = irEmitter.emitInsertValue(methodValue, hashValue, { 1 });
			return methodValue;
		}
		
	}
	
}

