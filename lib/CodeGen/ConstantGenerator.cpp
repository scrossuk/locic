#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace CodeGen {
	
		ConstantGenerator::ConstantGenerator(Module& module)
			: module_(module) { }
		
		llvm::UndefValue* ConstantGenerator::getVoidUndef() const {
			return llvm::UndefValue::get(
				llvm::Type::getVoidTy(module_.getLLVMContext()));
		}
		
		llvm::UndefValue* ConstantGenerator::getUndef(llvm::Type* type) const {
			return llvm::UndefValue::get(type);
		}
		
		llvm::Constant* ConstantGenerator::getNull(llvm::Type* type) const {
			return llvm::Constant::getNullValue(type);
		}
		
		llvm::ConstantPointerNull* ConstantGenerator::getNullPointer() const {
			return llvm::ConstantPointerNull::get(TypeGenerator(module_).getPtrType());
		}
		
		llvm::ConstantInt* ConstantGenerator::getBool(bool value) const {
			return getI8(value ? 1 : 0);
		}
		
		llvm::ConstantInt* ConstantGenerator::getInt(size_t sizeInBits, long long intValue) const {
			assert(sizeInBits >= 1);
			return llvm::ConstantInt::get(module_.getLLVMContext(),
						      llvm::APInt(sizeInBits, intValue));
		}
		
		llvm::ConstantInt* ConstantGenerator::getIntByType(llvm::IntegerType* type, long long intValue) const {
			return llvm::ConstantInt::get(type, intValue);
		}
		
		llvm::ConstantInt* ConstantGenerator::getI1(bool value) const {
			return getInt(1, value ? 1 : 0);
		}
		
		llvm::ConstantInt* ConstantGenerator::getI8(uint8_t value) const {
			return getInt(8, value);
		}
		
		llvm::ConstantInt* ConstantGenerator::getI16(uint16_t value) const {
			return getInt(32, value);
		}
		
		llvm::ConstantInt* ConstantGenerator::getI32(uint32_t value) const {
			return getInt(32, value);
		}
		
		llvm::ConstantInt* ConstantGenerator::getI64(uint64_t value) const {
			return getInt(64, value);
		}
		
		llvm::ConstantInt* ConstantGenerator::getSizeTValue(const unsigned long long sizeValue) const {
			const auto& abiTypeInfo = module_.abi().typeInfo();
			const auto sizeTypeWidth = abiTypeInfo.getTypeRawSize(getBasicPrimitiveABIType(module_, PrimitiveSize));
			return getInt(sizeTypeWidth.asBits(), sizeValue);
		}
		
		llvm::ConstantInt* ConstantGenerator::getPrimitiveInt(const PrimitiveID primitiveID,
		                                                      const long long intValue) const {
			const auto& abiTypeInfo = module_.abi().typeInfo();
			const auto primitiveWidth = abiTypeInfo.getTypeRawSize(getBasicPrimitiveABIType(module_, primitiveID));
			return getInt(primitiveWidth.asBits(), intValue);
		}
		
		llvm::Constant* ConstantGenerator::getPrimitiveFloat(const PrimitiveID primitiveID,
		                                                     const long double floatValue) const {
			switch (primitiveID) {
				case PrimitiveFloat:
					return getFloat(floatValue);
				case PrimitiveDouble:
					return getDouble(floatValue);
				case PrimitiveLongDouble:
					return getLongDouble(floatValue);
				default:
					llvm_unreachable("Unknown constant primitive float type name.");
			}
		}
		
		llvm::Constant* ConstantGenerator::getFloat(const float value) const {
			return llvm::ConstantFP::get(module_.getLLVMContext(), llvm::APFloat(value));
		}
		
		llvm::Constant* ConstantGenerator::getDouble(const double value) const {
			return llvm::ConstantFP::get(module_.getLLVMContext(), llvm::APFloat(value));
		}
		
		llvm::Constant* ConstantGenerator::getLongDouble(const long double value) const {
			const auto& abiTypeInfo = module_.abi().typeInfo();
			return llvm::ConstantFP::get(abiTypeInfo.getLLVMType(llvm_abi::LongDoubleTy), value);
		}
		
		llvm::Constant* ConstantGenerator::getArray(llvm::ArrayType* arrayType, llvm::ArrayRef<llvm::Constant*> values) const {
			return llvm::ConstantArray::get(arrayType, values);
		}
		
		llvm::Constant* ConstantGenerator::getStruct(llvm::StructType* structType, llvm::ArrayRef<llvm::Constant*> values) const {
			return llvm::ConstantStruct::get(structType, values);
		}
		
		llvm::Constant* ConstantGenerator::getString(const String& value, bool withNullTerminator) const {
			return llvm::ConstantDataArray::getString(module_.getLLVMContext(),
								  value.c_str(), withNullTerminator);
		}
		
		llvm::Constant* ConstantGenerator::getPointerCast(llvm::Constant* value, llvm::Type* targetType) const {
			assert(value->getType()->isPointerTy());
			assert(targetType->isPointerTy());
			return llvm::ConstantExpr::getPointerCast(value, targetType);
		}
		
		llvm::Constant* ConstantGenerator::getAlignOf(llvm::Type* type) const {
			return llvm::ConstantExpr::getAlignOf(type);
		}
		
		llvm::Constant* ConstantGenerator::getSizeOf(llvm::Type* type) const {
			return llvm::ConstantExpr::getSizeOf(type);
		}
		
		llvm::Constant* ConstantGenerator::getGetElementPtr(llvm::Type* const type,
		                                                    llvm::Constant* const operand,
		                                                    llvm::ArrayRef<llvm::Constant*> args) const {
			const auto castOperand = getPointerCast(operand,
			                                        type->getPointerTo());
#if LOCIC_LLVM_VERSION >= 307
			return llvm::ConstantExpr::getGetElementPtr(type,
			                                            castOperand,
			                                            args);
#else
			return llvm::ConstantExpr::getGetElementPtr(castOperand,
			                                            args);
#endif
		}
		
		llvm::Constant* ConstantGenerator::getExtractValue(llvm::Constant* operand, llvm::ArrayRef<unsigned> args) const {
			return llvm::ConstantExpr::getExtractValue(operand, args);
		}
		
		llvm::Constant* ConstantGenerator::getMin(llvm::Constant* first, llvm::Constant* second) const {
			llvm::Constant* compareResult = llvm::ConstantExpr::getICmp(llvm::CmpInst::ICMP_ULT, first, second);
			return llvm::ConstantExpr::getSelect(compareResult, first, second);
		}
		
		llvm::Constant* ConstantGenerator::getMax(llvm::Constant* first, llvm::Constant* second) const {
			llvm::Constant* compareResult = llvm::ConstantExpr::getICmp(llvm::CmpInst::ICMP_UGT, first, second);
			return llvm::ConstantExpr::getSelect(compareResult, first, second);
		}
		
	}
	
}
