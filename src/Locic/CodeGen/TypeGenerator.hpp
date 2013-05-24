#ifndef LOCIC_CODEGEN_TYPEGENERATOR_HPP
#define LOCIC_CODEGEN_TYPEGENERATOR_HPP

#include <vector>

#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
	
		class TypeGenerator {
			public:
				inline TypeGenerator(const Module& module)
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
				
				inline llvm::IntegerType* getI32Type() const {
					return llvm::Type::getInt32Ty(module_.getLLVMContext());
				}
				
				inline llvm::IntegerType* getIntType(size_t typeSizeInBits) const {
					return llvm::IntegerType::get(module_.getLLVMContext(), typeSizeInBits);
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
					return llvm::Type::getFP128Ty(module_.getLLVMContext());
				}
				
				inline llvm::ArrayType* getArrayType(llvm::Type* elementType, size_t size) const {
					return llvm::ArrayType::get(elementType, size);
				}
				
				inline llvm::FunctionType* getVoidFunctionType(const std::vector<llvm::Type*>& args) const {
					const bool isVarArg = false;
					return llvm::FunctionType::get(getVoidType(), args, isVarArg);
				}
				
				inline llvm::FunctionType* getFunctionType(llvm::Type* returnType, const std::vector<llvm::Type*>& args, bool isVarArg = false) const {
					return llvm::FunctionType::get(returnType, args, isVarArg);
				}
				
				inline llvm::FunctionType* getVarArgsFunctionType(llvm::Type* returnType, const std::vector<llvm::Type*>& args) const {
					const bool isVarArg = true;
					return getFunctionType(returnType, args, isVarArg);
				}
				
				inline llvm::StructType* getStructType(const std::vector<llvm::Type*>& members) const {
					return llvm::StructType::get(module_.getLLVMContext(), members);
				}
				
				inline llvm::StructType* getForwardDeclaredStructType(const std::string& name) const {
					return llvm::StructType::create(module_.getLLVMContext(), name);
				}
				
			private:
				const Module& module_;
				
		};
		
	}
	
}

#endif
