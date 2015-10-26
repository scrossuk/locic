#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		TypeGenerator::TypeGenerator(Module& module)
		: module_(module) { }
		
		llvm::Type* TypeGenerator::getVoidType() const {
			return llvm::Type::getVoidTy(module_.getLLVMContext());
		}
		
		llvm::IntegerType* TypeGenerator::getI1Type() const {
			return llvm::Type::getInt1Ty(module_.getLLVMContext());
		}
		
		llvm::IntegerType* TypeGenerator::getI8Type() const {
			return llvm::Type::getInt8Ty(module_.getLLVMContext());
		}
		
		llvm::IntegerType* TypeGenerator::getI16Type() const {
			return llvm::Type::getInt16Ty(module_.getLLVMContext());
		}
		
		llvm::IntegerType* TypeGenerator::getI32Type() const {
			return llvm::Type::getInt32Ty(module_.getLLVMContext());
		}
		
		llvm::IntegerType* TypeGenerator::getI64Type() const {
			return llvm::Type::getInt64Ty(module_.getLLVMContext());
		}
		
		llvm::IntegerType* TypeGenerator::getIntType(size_t typeSizeInBits) const {
			return llvm::IntegerType::get(module_.getLLVMContext(), typeSizeInBits);
		}
		
		llvm::IntegerType* TypeGenerator::getSizeTType() const {
			const auto abiType = getBasicPrimitiveABIType(module_, PrimitiveSize);
			const auto sizeTypeWidth = module_.abi().typeInfo().getTypeRawSize(abiType);
			return llvm::IntegerType::get(module_.getLLVMContext(),
			                              sizeTypeWidth.asBits());
		}
		
		llvm::PointerType* TypeGenerator::getPtrType() const {
			return llvm::Type::getInt8PtrTy(module_.getLLVMContext());
		}
		
		llvm::Type* TypeGenerator::getFloatType() const {
			return llvm::Type::getFloatTy(module_.getLLVMContext());
		}
		
		llvm::Type* TypeGenerator::getDoubleType() const {
			return llvm::Type::getDoubleTy(module_.getLLVMContext());
		}
		
		llvm::Type* TypeGenerator::getLongDoubleType() const {
			return module_.abi().typeInfo().getLLVMType(llvm_abi::LongDoubleTy);
		}
		
		llvm::ArrayType* TypeGenerator::getArrayType(llvm::Type* elementType, size_t size) const {
			return llvm::ArrayType::get(elementType, size);
		}
		
		llvm::FunctionType* TypeGenerator::getVoidFunctionType(llvm::ArrayRef<llvm::Type*> args) const {
			const bool isVarArg = false;
			return llvm::FunctionType::get(getVoidType(), args, isVarArg);
		}
		
		llvm::FunctionType* TypeGenerator::getFunctionType(llvm::Type* returnType, llvm::ArrayRef<llvm::Type*> args, bool isVarArg) const {
			return llvm::FunctionType::get(returnType, args, isVarArg);
		}
		
		llvm::FunctionType* TypeGenerator::getVarArgsFunctionType(llvm::Type* returnType, llvm::ArrayRef<llvm::Type*> args) const {
			const bool isVarArg = true;
			return getFunctionType(returnType, args, isVarArg);
		}
		
		llvm::StructType* TypeGenerator::getStructType(llvm::ArrayRef<llvm::Type*> members) const {
			return llvm::StructType::get(module_.getLLVMContext(), members);
		}
		
		llvm::StructType* TypeGenerator::getOpaqueStructType() const {
			return llvm::StructType::create(module_.getLLVMContext());
		}
		
		llvm::StructType* TypeGenerator::getForwardDeclaredStructType(const String& name) const {
			return llvm::StructType::create(module_.getLLVMContext(), name.c_str());
		}
		
	}

}
