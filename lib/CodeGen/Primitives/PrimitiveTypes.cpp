#include <assert.h>

#include <stdexcept>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>

#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace CodeGen {
		
		llvm::Type* getPrimitiveType(Module& module, const SEM::Type* const type) {
			const auto& primitive = module.getPrimitive(*(type->getObjectType()));
			return primitive.getIRType(module,
			                           TypeGenerator(module),
			                           arrayRef(type->templateArguments()));
		}
		
		llvm_abi::Type getPrimitiveABIIntegerType(const PrimitiveID id) {
			switch (id) {
				case PrimitiveCompareResult:
					// Compare results represented with 8 bits.
					return llvm_abi::Int8Ty;
				case PrimitiveBool:
					return llvm_abi::BoolTy;
				case PrimitiveInt8:
					return llvm_abi::Int8Ty;
				case PrimitiveUInt8:
					return llvm_abi::UInt8Ty;
				case PrimitiveInt16:
					return llvm_abi::Int16Ty;
				case PrimitiveUInt16:
					return llvm_abi::UInt16Ty;
				case PrimitiveInt32:
					return llvm_abi::Int32Ty;
				case PrimitiveUInt32:
					return llvm_abi::UInt32Ty;
				case PrimitiveInt64:
					return llvm_abi::Int64Ty;
				case PrimitiveUInt64:
					return llvm_abi::UInt64Ty;
				case PrimitiveByte:
					return llvm_abi::CharTy;
				case PrimitiveUByte:
					return llvm_abi::UCharTy;
				case PrimitiveShort:
					return llvm_abi::ShortTy;
				case PrimitiveUShort:
					return llvm_abi::UShortTy;
				case PrimitiveInt:
					return llvm_abi::IntTy;
				case PrimitiveUInt:
					return llvm_abi::UIntTy;
				case PrimitiveLong:
					return llvm_abi::LongTy;
				case PrimitiveULong:
					return llvm_abi::ULongTy;
				case PrimitiveLongLong:
					return llvm_abi::LongLongTy;
				case PrimitiveULongLong:
					return llvm_abi::ULongLongTy;
				case PrimitiveSize:
					return llvm_abi::SizeTy;
				case PrimitiveSSize:
					return llvm_abi::SSizeTy;
				case PrimitivePtrDiff:
					return llvm_abi::PtrDiffTy;
				default:
					llvm_unreachable("Primitive type is not an integer.");
			}
		}
		
		llvm::Type* getBasicPrimitiveType(Module& module, const PrimitiveID id) {
			switch (id) {
				case PrimitiveVoid:
					return TypeGenerator(module).getVoidType();
				case PrimitiveNull:
					return TypeGenerator(module).getPtrType();
				case PrimitiveCompareResult:
					return TypeGenerator(module).getI8Type();
				case PrimitiveBool:
					return TypeGenerator(module).getI1Type();
				case PrimitiveInt8:
				case PrimitiveUInt8:
				case PrimitiveInt16:
				case PrimitiveUInt16:
				case PrimitiveInt32:
				case PrimitiveUInt32:
				case PrimitiveInt64:
				case PrimitiveUInt64:
				case PrimitiveByte:
				case PrimitiveUByte:
				case PrimitiveShort:
				case PrimitiveUShort:
				case PrimitiveInt:
				case PrimitiveUInt:
				case PrimitiveLong:
				case PrimitiveULong:
				case PrimitiveLongLong:
				case PrimitiveULongLong:
				case PrimitiveSize:
				case PrimitiveSSize:
				case PrimitivePtrDiff: {
					const auto intAbiType = getPrimitiveABIIntegerType(id);
					const auto typeSize = module.abi().typeInfo().getTypeRawSize(intAbiType);
					return TypeGenerator(module).getIntType(typeSize.asBits());
				}
				case PrimitiveFloat:
					return TypeGenerator(module).getFloatType();
				case PrimitiveDouble:
					return TypeGenerator(module).getDoubleType();
				case PrimitiveLongDouble:
					return TypeGenerator(module).getLongDoubleType();
				case PrimitivePtr:
				case PrimitivePtrLval:
				CASE_CALLABLE_ID(PrimitiveFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveMethodFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveVarArgFunctionPtr):
					return TypeGenerator(module).getPtrType();
				CASE_CALLABLE_ID(PrimitiveTemplatedFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveTemplatedMethodFunctionPtr): {
					llvm::Type* const memberTypes[] = {
						TypeGenerator(module).getPtrType(),
						templateGeneratorType(module).second
					};
					return TypeGenerator(module).getStructType(memberTypes);
				}
				CASE_CALLABLE_ID(PrimitiveMethod): {
					llvm::Type* const memberTypes[] = {
						TypeGenerator(module).getPtrType(),
						getBasicPrimitiveType(module, PrimitiveMethodFunctionPtr0)
					};
					return TypeGenerator(module).getStructType(memberTypes);
				}
				CASE_CALLABLE_ID(PrimitiveTemplatedMethod): {
					llvm::Type* const memberTypes[] = {
						TypeGenerator(module).getPtrType(),
						getBasicPrimitiveType(module, PrimitiveTemplatedMethodFunctionPtr0)
					};
					return TypeGenerator(module).getStructType(memberTypes);
				}
				CASE_CALLABLE_ID(PrimitiveInterfaceMethod): {
					return interfaceMethodType(module).second;
				}
				CASE_CALLABLE_ID(PrimitiveStaticInterfaceMethod): {
					return staticInterfaceMethodType(module).second;
				}
				case PrimitiveTypename:
					return typeInfoType(module).second;
				default:
					llvm_unreachable("Unrecognised primitive type.");
			}
		}
		
		llvm_abi::Type getBasicPrimitiveABIType(Module& module, const PrimitiveID id) {
			auto& abiTypeBuilder = module.abiTypeBuilder();
			
			switch (id) {
				case PrimitiveVoid:
					return llvm_abi::VoidTy;
				case PrimitiveNull:
				case PrimitivePtr:
				CASE_CALLABLE_ID(PrimitiveFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveMethodFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveVarArgFunctionPtr):
					return llvm_abi::PointerTy;
				CASE_CALLABLE_ID(PrimitiveTemplatedFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveTemplatedMethodFunctionPtr): {
					std::vector<llvm_abi::Type> types;
					types.reserve(2);
					types.push_back(llvm_abi::PointerTy);
					types.push_back(templateGeneratorType(module).first);
					return llvm_abi::Type::AutoStruct(abiTypeBuilder, types);
				}
				CASE_CALLABLE_ID(PrimitiveMethod): {
					std::vector<llvm_abi::Type> types;
					types.reserve(2);
					types.push_back(llvm_abi::PointerTy);
					types.push_back(getBasicPrimitiveABIType(module, PrimitiveMethodFunctionPtr0));
					return llvm_abi::Type::AutoStruct(abiTypeBuilder, types);
				}
				CASE_CALLABLE_ID(PrimitiveTemplatedMethod): {
					std::vector<llvm_abi::Type> types;
					types.reserve(2);
					types.push_back(llvm_abi::PointerTy);
					types.push_back(getBasicPrimitiveABIType(module, PrimitiveTemplatedMethodFunctionPtr0));
					return llvm_abi::Type::AutoStruct(abiTypeBuilder, types);
				}
				CASE_CALLABLE_ID(PrimitiveInterfaceMethod): {
					return interfaceMethodType(module).first;
				}
				CASE_CALLABLE_ID(PrimitiveStaticInterfaceMethod): {
					return staticInterfaceMethodType(module).first;
				}
				case PrimitiveCompareResult:
				case PrimitiveBool:
				case PrimitiveInt8:
				case PrimitiveUInt8:
				case PrimitiveInt16:
				case PrimitiveUInt16:
				case PrimitiveInt32:
				case PrimitiveUInt32:
				case PrimitiveInt64:
				case PrimitiveUInt64:
				case PrimitiveByte:
				case PrimitiveUByte:
				case PrimitiveShort:
				case PrimitiveUShort:
				case PrimitiveInt:
				case PrimitiveUInt:
				case PrimitiveLong:
				case PrimitiveULong:
				case PrimitiveLongLong:
				case PrimitiveULongLong:
				case PrimitiveSize:
				case PrimitiveSSize:
				case PrimitivePtrDiff:
					return getPrimitiveABIIntegerType(id);
				case PrimitiveFloat:
					return llvm_abi::FloatTy;
				case PrimitiveDouble:
					return llvm_abi::DoubleTy;
				case PrimitiveLongDouble:
					return llvm_abi::LongDoubleTy;
				default:
					llvm_unreachable("Unrecognised primitive type.");
			}
		}
		
		llvm_abi::Type getPrimitiveABIType(Module& module, const SEM::Type* const type) {
			assert(TypeInfo(module).isSizeKnownInThisModule(type));
			
			const auto& primitive = module.getPrimitive(*(type->getObjectType()));
			return primitive.getABIType(module,
			                            module.abiTypeBuilder(),
			                            arrayRef(type->templateArguments()));
		}
		
	}
	
}

