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
		
		llvm::Type* interfaceStructLLVMType(Module& module) {
			TypeGenerator typeGen(module);
			llvm::Type* const memberTypes[] = { typeGen.getPtrType(), typeInfoType(module).second };
			return typeGen.getStructType(memberTypes);
		}
		
		llvm_abi::Type* interfaceStructABIType(Module& module) {
			auto& abiContext = module.abiContext();
			llvm::SmallVector<llvm_abi::Type*, 2> types;
			types.push_back(llvm_abi::Type::Pointer(abiContext));
			types.push_back(typeInfoType(module).first);
			return llvm_abi::Type::AutoStruct(abiContext, types);
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
		
		llvm::Type* interfaceMethodLLVMType(Module& module) {
			TypeGenerator typeGen(module);
			llvm::Type* const memberTypes[] = { interfaceStructLLVMType(module), typeGen.getI64Type() };
			return typeGen.getStructType(memberTypes);
		}
		
		llvm_abi::Type* interfaceMethodABIType(Module& module) {
			auto& abiContext = module.abiContext();
			llvm::SmallVector<llvm_abi::Type*, 2> types;
			types.push_back(interfaceStructABIType(module));
			types.push_back(llvm_abi::Type::Integer(abiContext, llvm_abi::Int64));
			return llvm_abi::Type::AutoStruct(abiContext, types);
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
		
		llvm::Type* staticInterfaceMethodLLVMType(Module& module) {
			TypeGenerator typeGen(module);
			llvm::Type* const memberTypes[] = { typeInfoType(module).second, typeGen.getI64Type() };
			return typeGen.getStructType(memberTypes);
		}
		
		llvm_abi::Type* staticInterfaceMethodABIType(Module& module) {
			auto& abiContext = module.abiContext();
			llvm::SmallVector<llvm_abi::Type*, 2> types;
			types.push_back(typeInfoType(module).first);
			types.push_back(llvm_abi::Type::Integer(abiContext, llvm_abi::Int64));
			return llvm_abi::Type::AutoStruct(abiContext, types);
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

