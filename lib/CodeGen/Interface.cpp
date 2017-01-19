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
			auto typeInfoValue = irEmitter.getUndef(typeInfoType(function.module()));
			typeInfoValue = irEmitter.emitInsertValue(typeInfoValue, vtablePointer, { 0 });
			typeInfoValue = irEmitter.emitInsertValue(typeInfoValue, templateGenerator, { 1 });
			return typeInfoValue;
		}
		
		llvm_abi::Type interfaceStructType(Module& module) {
			llvm::SmallVector<llvm_abi::Type, 2> types;
			types.push_back(llvm_abi::PointerTy);
			types.push_back(typeInfoType(module));
			return module.abiTypeBuilder().getStructTy(types, "__interface_ref");
		}
		
		llvm::Value* makeInterfaceStructValue(Function& function, llvm::Value* contextPointer, llvm::Value* typeInfoValue) {
			IREmitter irEmitter(function);
			auto interfaceValue = irEmitter.getUndef(interfaceStructType(function.module()));
			interfaceValue = irEmitter.emitInsertValue(interfaceValue, contextPointer, { 0 });
			interfaceValue = irEmitter.emitInsertValue(interfaceValue, typeInfoValue, { 1 });
			return interfaceValue;
		}
		
		llvm_abi::Type interfaceMethodType(Module& module) {
			llvm::SmallVector<llvm_abi::Type, 2> types;
			types.push_back(interfaceStructType(module));
			types.push_back(llvm_abi::Int64Ty);
			return module.abiTypeBuilder().getStructTy(types, "__interface_method");
		}
		
		llvm::Value* makeInterfaceMethodValue(Function& function, llvm::Value* interfaceStructValue, llvm::Value* hashValue) {
			IREmitter irEmitter(function);
			auto methodValue = irEmitter.getUndef(interfaceMethodType(function.module()));
			methodValue = irEmitter.emitInsertValue(methodValue, interfaceStructValue, { 0 });
			methodValue = irEmitter.emitInsertValue(methodValue, hashValue, { 1 });
			return methodValue;
		}
		
		llvm_abi::Type staticInterfaceMethodType(Module& module) {
			llvm::SmallVector<llvm_abi::Type, 2> types;
			types.push_back(typeInfoType(module));
			types.push_back(llvm_abi::Int64Ty);
			return module.abiTypeBuilder().getStructTy(types, "__static_interface_method");
		}
		
		llvm::Value* makeStaticInterfaceMethodValue(Function& function, llvm::Value* typeInfoValue, llvm::Value* hashValue) {
			IREmitter irEmitter(function);
			auto methodValue = irEmitter.getUndef(staticInterfaceMethodType(function.module()));
			methodValue = irEmitter.emitInsertValue(methodValue, typeInfoValue, { 0 });
			methodValue = irEmitter.emitInsertValue(methodValue, hashValue, { 1 });
			return methodValue;
		}
		
	}
	
}

