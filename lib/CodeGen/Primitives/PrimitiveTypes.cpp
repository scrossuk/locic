#include <assert.h>

#include <stdexcept>
#include <string>

#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Type* getPrimitiveType(Module& module, const SEM::Type* type) {
			const auto& name = type->getObjectType()->name().last();
			const auto kind = module.primitiveKind(name);
			
			switch (kind) {
				case PrimitiveRef: {
					if (type->templateArguments().at(0)->isInterface()) {
						return interfaceStructType(module).second;
					} else {
						return genPointerType(module, type->templateArguments().at(0));
					}
				}
				case PrimitivePtr:
				case PrimitivePtrLval:
					return genPointerType(module, type->templateArguments().at(0));
				case PrimitiveValueLval: {
					if (isPrimitiveTypeSizeKnownInThisModule(module, type)) {
						TypeGenerator typeGen(module);
						const auto targetType = genType(module, type->templateArguments().at(0));
						llvm::Type* const types[] = { targetType, typeGen.getI1Type() };
						return typeGen.getStructType(types);
					}
					break;
				}
				case PrimitiveMemberLval:
				case PrimitiveFinalLval: {
					if (isPrimitiveTypeSizeKnownInThisModule(module, type)) {
						return genType(module, type->templateArguments().at(0));
					}
					break;
				}
				default:
					break;
			}
			
			return getBasicPrimitiveType(module, kind);
		}
		
		llvm_abi::IntegerKind primitiveABIIntegerKind(PrimitiveKind kind) {
			switch (kind) {
				case PrimitiveCompareResult:
					// Compare results represented with 8 bits.
					return llvm_abi::Int8;
				case PrimitiveBool:
					return llvm_abi::Bool;
				case PrimitiveInt8:
				case PrimitiveUInt8:
					return llvm_abi::Int8;
				case PrimitiveInt16:
				case PrimitiveUInt16:
					return llvm_abi::Int16;
				case PrimitiveInt32:
				case PrimitiveUInt32:
					return llvm_abi::Int32;
				case PrimitiveInt64:
				case PrimitiveUInt64:
					return llvm_abi::Int64;
				case PrimitiveByte:
				case PrimitiveUByte:
					return llvm_abi::Char;
				case PrimitiveShort:
				case PrimitiveUShort:
					return llvm_abi::Short;
				case PrimitiveInt:
				case PrimitiveUInt:
					return llvm_abi::Int;
				case PrimitiveLong:
				case PrimitiveULong:
					return llvm_abi::Long;
				case PrimitiveLongLong:
				case PrimitiveULongLong:
					return llvm_abi::LongLong;
				case PrimitiveSize:
				case PrimitiveSSize:
					return llvm_abi::SizeT;
				case PrimitivePtrDiff:
					return llvm_abi::PtrDiffT;
				default:
					llvm_unreachable("Primitive type is not an integer.");
			}
		}
		
		llvm::Type* getLvalStruct(Module& module, const std::string& name) {
			const auto existingType = module.getTypeMap().tryGet(name);
			if (existingType) {
				return *existingType;
			}
			
			const auto type = TypeGenerator(module).getForwardDeclaredStructType(name);
			module.getTypeMap().insert(name, type);
			return type;
		}
		
		llvm::Type* getBasicPrimitiveType(Module& module, PrimitiveKind kind) {
			switch (kind) {
				case PrimitiveVoid:
					return TypeGenerator(module).getVoidType();
				case PrimitiveNull:
					return TypeGenerator(module).getI8PtrType();
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
					const auto intAbiType = llvm_abi::Type::Integer(module.abiContext(), primitiveABIIntegerKind(kind));
					return TypeGenerator(module).getIntType(module.abi().typeSize(intAbiType) * 8);
				}
				case PrimitiveFloat:
					return TypeGenerator(module).getFloatType();
				case PrimitiveDouble:
					return TypeGenerator(module).getDoubleType();
				case PrimitiveLongDouble:
					return TypeGenerator(module).getLongDoubleType();
				case PrimitivePtr:
				case PrimitivePtrLval:
					return TypeGenerator(module).getI8PtrType();
				case PrimitiveValueLval:
					return getLvalStruct(module, "value_lval");
				case PrimitiveMemberLval:
					return getLvalStruct(module, "member_lval");
				case PrimitiveFinalLval:
					return getLvalStruct(module, "final_lval");
				case PrimitiveTypename:
					return typeInfoType(module).second;
				default:
					llvm_unreachable("Unrecognised primitive type.");
			}
		}
		
		llvm::Type* getNamedPrimitiveType(Module& module, const std::string& name) {
			return getBasicPrimitiveType(module, module.primitiveKind(name));
		}
		
		llvm_abi::Type* getBasicPrimitiveABIType(Module& module, PrimitiveKind kind) {
			auto& abiContext = module.abiContext();
			
			switch (kind) {
				case PrimitiveVoid:
					// TODO: use a void type?
					return llvm_abi::Type::Struct(abiContext, {});
				case PrimitiveNull:
					return llvm_abi::Type::Pointer(abiContext);
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
					return llvm_abi::Type::Integer(abiContext, primitiveABIIntegerKind(kind));
				case PrimitiveFloat:
					return llvm_abi::Type::FloatingPoint(abiContext, llvm_abi::Float);
				case PrimitiveDouble:
					return llvm_abi::Type::FloatingPoint(abiContext, llvm_abi::Double);
				case PrimitiveLongDouble:
					return llvm_abi::Type::FloatingPoint(abiContext, llvm_abi::LongDouble);
				default:
					llvm_unreachable("Unrecognised primitive type.");
			}
		}
		
		llvm_abi::Type* getNamedPrimitiveABIType(Module& module, const std::string& name) {
			return getBasicPrimitiveABIType(module, module.primitiveKind(name));
		}
		
		llvm_abi::Type* getPrimitiveABIType(Module& module, const SEM::Type* type) {
			assert(isTypeSizeKnownInThisModule(module, type));
			
			const auto typeInstance = type->getObjectType();
			const auto& name = typeInstance->name().last();
			
			auto& abiContext = module.abiContext();
			const auto kind = module.primitiveKind(name);
			
			switch (kind) {
				case PrimitiveRef: {
					if (type->templateArguments().at(0)->isInterface()) {
						return interfaceStructType(module).first;
					} else {
						return llvm_abi::Type::Pointer(abiContext);
					}
				}
				case PrimitivePtr:
				case PrimitivePtrLval:
					return llvm_abi::Type::Pointer(abiContext);
				case PrimitiveValueLval: {
					llvm::SmallVector<llvm_abi::Type*, 2> members;
					members.push_back(genABIType(module, type->templateArguments().at(0)));
					members.push_back(llvm_abi::Type::Integer(abiContext, llvm_abi::Bool));
					return llvm_abi::Type::AutoStruct(abiContext, members);
				}
				case PrimitiveFinalLval:
				case PrimitiveMemberLval:
					return genABIType(module, type->templateArguments().at(0));
				case PrimitiveTypename:
					return typeInfoType(module).first;
				default:
					return getBasicPrimitiveABIType(module, kind);
			}
		}
		
	}
	
}

