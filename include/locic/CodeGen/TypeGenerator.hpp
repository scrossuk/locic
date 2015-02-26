#ifndef LOCIC_CODEGEN_TYPEGENERATOR_HPP
#define LOCIC_CODEGEN_TYPEGENERATOR_HPP

#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		class Module;
		
		class TypeGenerator {
			public:
				inline TypeGenerator(Module& module)
					: module_(module) { }
					
				inline llvm::Type* getVoidType() const {
					return llvm::Type::getVoidTy(module_.getLLVMContext());
				}
				
				inline llvm::IntegerType* getI1Type() const {
					return llvm::Type::getInt1Ty(module_.getLLVMContext());
				}
				
				inline llvm::IntegerType* getI8Type() const {
					return llvm::Type::getInt8Ty(module_.getLLVMContext());
				}
				
				inline llvm::IntegerType* getI16Type() const {
					return llvm::Type::getInt16Ty(module_.getLLVMContext());
				}
				
				inline llvm::IntegerType* getI32Type() const {
					return llvm::Type::getInt32Ty(module_.getLLVMContext());
				}
				
				inline llvm::IntegerType* getI64Type() const {
					return llvm::Type::getInt64Ty(module_.getLLVMContext());
				}
				
				inline llvm::IntegerType* getIntType(size_t typeSizeInBits) const {
					return llvm::IntegerType::get(module_.getLLVMContext(), typeSizeInBits);
				}
				
				inline llvm::IntegerType* getSizeTType() const {
					const size_t sizeTypeWidth = module_.abi().typeSize(getBasicPrimitiveABIType(module_, PrimitiveSize));
					return llvm::IntegerType::get(module_.getLLVMContext(), sizeTypeWidth * 8);
				}
				
				inline llvm::PointerType* getI8PtrType() const {
					return llvm::Type::getInt8PtrTy(module_.getLLVMContext());
				}
				
				inline llvm::Type* getFloatType() const {
					return llvm::Type::getFloatTy(module_.getLLVMContext());
				}
				
				inline llvm::Type* getDoubleType() const {
					return llvm::Type::getDoubleTy(module_.getLLVMContext());
				}
				
				inline llvm::Type* getLongDoubleType() const {
					return module_.abi().longDoubleType();
				}
				
				inline llvm::ArrayType* getArrayType(llvm::Type* elementType, size_t size) const {
					return llvm::ArrayType::get(elementType, size);
				}
				
				inline llvm::FunctionType* getVoidFunctionType(llvm::ArrayRef<llvm::Type*> args) const {
					const bool isVarArg = false;
					return llvm::FunctionType::get(getVoidType(), args, isVarArg);
				}
				
				inline llvm::FunctionType* getFunctionType(llvm::Type* returnType, llvm::ArrayRef<llvm::Type*> args, bool isVarArg = false) const {
					return llvm::FunctionType::get(returnType, args, isVarArg);
				}
				
				inline llvm::FunctionType* getVarArgsFunctionType(llvm::Type* returnType, llvm::ArrayRef<llvm::Type*> args) const {
					const bool isVarArg = true;
					return getFunctionType(returnType, args, isVarArg);
				}
				
				inline llvm::StructType* getStructType(llvm::ArrayRef<llvm::Type*> members) const {
					return llvm::StructType::get(module_.getLLVMContext(), members);
				}
				
				inline llvm::StructType* getOpaqueStructType() const {
					return llvm::StructType::create(module_.getLLVMContext());
				}
				
				inline llvm::StructType* getForwardDeclaredStructType(const String& name) const {
					return llvm::StructType::create(module_.getLLVMContext(), name.c_str());
				}
				
			private:
				Module& module_;
				
		};
		
	}
	
}

#endif
