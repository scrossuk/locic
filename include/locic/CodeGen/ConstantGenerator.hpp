#ifndef LOCIC_CODEGEN_CONSTANTGENERATOR_HPP
#define LOCIC_CODEGEN_CONSTANTGENERATOR_HPP

#include <stdint.h>

#include <locic/CodeGen/LLVMIncludes.hpp>

namespace locic {
	
	class PrimitiveID;
	class String;
	
	namespace CodeGen {
	
		class Module;
		
		class ConstantGenerator {
			public:
				ConstantGenerator(Module& module);
				
				llvm::UndefValue* getVoidUndef() const;
				
				llvm::UndefValue* getUndef(llvm::Type* type) const;
				
				llvm::Constant* getNull(llvm::Type* type) const;
				
				llvm::ConstantPointerNull* getNullPointer() const;
				
				llvm::ConstantInt* getBool(bool value) const;
				
				llvm::ConstantInt* getInt(size_t sizeInBits, long long intValue) const;
				
				llvm::ConstantInt* getIntByType(llvm::IntegerType* type, long long intValue) const;
				
				llvm::ConstantInt* getI1(bool value) const;
				
				llvm::ConstantInt* getI8(uint8_t value) const;
				
				llvm::ConstantInt* getI16(uint16_t value) const;
				
				llvm::ConstantInt* getI32(uint32_t value) const;
				
				llvm::ConstantInt* getI64(uint64_t value) const;
				
				llvm::ConstantInt* getSizeTValue(unsigned long long sizeValue) const;
				
				llvm::ConstantInt* getPrimitiveInt(PrimitiveID primitiveID, long long intValue) const;
				
				llvm::Constant* getPrimitiveFloat(PrimitiveID primitiveID, long double floatValue) const;
				
				llvm::Constant* getFloat(float value) const;
				
				llvm::Constant* getDouble(double value) const;
				
				llvm::Constant* getLongDouble(long double value) const;
				
				llvm::Constant* getArray(llvm::ArrayType* arrayType, llvm::ArrayRef<llvm::Constant*> values) const;
				
				llvm::Constant* getStruct(llvm::StructType* structType, llvm::ArrayRef<llvm::Constant*> values) const;
				
				llvm::Constant* getString(const String& value, bool withNullTerminator = true) const;
				
				llvm::Constant* getPointerCast(llvm::Constant* value, llvm::Type* targetType) const;
				
				llvm::Constant* getAlignOf(llvm::Type* type) const;
				
				llvm::Constant* getSizeOf(llvm::Type* type) const;
				
				llvm::Constant* getGetElementPtr(llvm::Type* type, llvm::Constant* operand, llvm::ArrayRef<llvm::Constant*> args) const;
				
				llvm::Constant* getExtractValue(llvm::Constant* operand, llvm::ArrayRef<unsigned> args) const;
				
				llvm::Constant* getMin(llvm::Constant* first, llvm::Constant* second) const;
				
				llvm::Constant* getMax(llvm::Constant* first, llvm::Constant* second) const;
				
			private:
				Module& module_;
				
		};
		
	}
	
}

#endif
