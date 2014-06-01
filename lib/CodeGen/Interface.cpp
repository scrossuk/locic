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
		
		llvm::Value* makeInterfaceMethodValue(Function& function, llvm::Value* interfaceStructValue, llvm::Value* hashValue) {
			llvm::Value* methodValue = ConstantGenerator(function.module()).getUndef(interfaceMethodType(function.module()));
			methodValue = function.getBuilder().CreateInsertValue(methodValue, interfaceStructValue, { 0 });
			methodValue = function.getBuilder().CreateInsertValue(methodValue, hashValue, { 1 });
			return methodValue;
		}
		
	}
	
}

