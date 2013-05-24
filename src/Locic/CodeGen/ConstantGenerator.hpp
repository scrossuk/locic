#ifndef LOCIC_CODEGEN_CONSTANTGENERATOR_HPP
#define LOCIC_CODEGEN_CONSTANTGENERATOR_HPP

#include <llvm/Constants.h>
#include <llvm/Type.h>

#include <Locic/CodeGen/Module.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>

namespace Locic {

	namespace CodeGen {
	
		class ConstantGenerator {
			public:
				inline ConstantGenerator(const Module& module)
					: module_(module) { }
				
				inline llvm::UndefValue* getVoidUndef() const {
					return llvm::UndefValue::get(
						llvm::Type::getVoidTy(module_.getLLVMContext()));
				}
				
				inline llvm::UndefValue* getUndef(llvm::Type* type) const {
					return llvm::UndefValue::get(type);
				}
				
				inline llvm::ConstantPointerNull* getNullPointer(llvm::PointerType* pointerType) const {
					return llvm::ConstantPointerNull::get(pointerType);
				}
				
				inline llvm::ConstantInt* getInt(size_t sizeInBits, long long intValue) const {
					assert(sizeInBits >= 1);
					return llvm::ConstantInt::get(module_.getLLVMContext(),
						llvm::APInt(sizeInBits, intValue));
				}
				
				inline llvm::ConstantInt* getI1(bool value) const {
					return getInt(1, value ? 1 : 0);
				}
				
				inline llvm::ConstantInt* getI32(uint32_t value) const {
					return getInt(32, value);
				}
				
				inline llvm::ConstantInt* getSize(unsigned long long sizeValue) const {
					const size_t sizeTypeWidth = module_.getTargetInfo().getPrimitiveSize("size_t");
					return getInt(sizeTypeWidth, sizeValue);
				}
				
				inline llvm::ConstantInt* getPrimitiveInt(const std::string& primitiveName, long long intValue) const {
					const size_t primitiveWidth = module_.getTargetInfo().getPrimitiveSize(primitiveName);
					return getInt(primitiveWidth, intValue);
				}
				
				inline llvm::ConstantFP* getFloat(float value) const {
					return llvm::ConstantFP::get(module_.getLLVMContext(),
						llvm::APFloat(value));
				}
				
				inline llvm::ConstantFP* getDouble(double value) const {
					return llvm::ConstantFP::get(module_.getLLVMContext(),
						llvm::APFloat(value));
				}
				
				inline llvm::Constant* getArray(llvm::ArrayType* arrayType, const std::vector<llvm::Constant*>& values) const {
					return llvm::ConstantArray::get(arrayType, values);
				}
				
				inline llvm::Constant* getStruct(llvm::StructType* structType, const std::vector<llvm::Constant*>& values) const {
					return llvm::ConstantStruct::get(structType, values);
				}
				
				inline llvm::Constant* getString(const std::string& value, bool withNullTerminator = true) const {
					return llvm::ConstantDataArray::getString(module_.getLLVMContext(),
						value, withNullTerminator);
				}
				
				inline llvm::Constant* getPointerCast(llvm::Constant* value, llvm::Type* targetType) const {
					return llvm::ConstantExpr::getPointerCast(value, targetType);
				}
				
			private:
				const Module& module_;
				
		};
		
	}
	
}

#endif
