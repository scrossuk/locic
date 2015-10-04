#include <assert.h>

#include <stdexcept>

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

#include <locic/Support/String.hpp>

namespace locic {
	
	namespace CodeGen {
		
		llvm::Type* getPrimitiveType(Module& module, const SEM::Type* const type) {
			const auto& primitive = module.getPrimitive(*(type->getObjectType()));
			return primitive.getIRType(module,
			                           TypeGenerator(module),
			                           arrayRef(type->templateArguments()));
		}
		
		llvm_abi::IntegerKind primitiveABIIntegerKind(const PrimitiveID id) {
			switch (id) {
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
					const auto intAbiType = llvm_abi::Type::Integer(module.abiContext(), primitiveABIIntegerKind(id));
					return TypeGenerator(module).getIntType(module.abi().typeSize(intAbiType) * 8);
				}
				case PrimitiveFloat:
					return TypeGenerator(module).getFloatType();
				case PrimitiveDouble:
					return TypeGenerator(module).getDoubleType();
				case PrimitiveLongDouble:
					return TypeGenerator(module).getLongDoubleType();
				case PrimitivePtr:
				case PrimitiveFunctionPtr:
				case PrimitiveMethodFunctionPtr:
				case PrimitiveVarArgFunctionPtr:
				case PrimitivePtrLval:
					return TypeGenerator(module).getPtrType();
				case PrimitiveTemplatedFunctionPtr:
				case PrimitiveTemplatedMethodFunctionPtr: {
					llvm::Type* const memberTypes[] = {
						TypeGenerator(module).getPtrType(),
						templateGeneratorType(module).second
					};
					return TypeGenerator(module).getStructType(memberTypes);
				}
				case PrimitiveMethod: {
					llvm::Type* const memberTypes[] = {
						TypeGenerator(module).getPtrType(),
						getBasicPrimitiveType(module, PrimitiveMethodFunctionPtr)
					};
					return TypeGenerator(module).getStructType(memberTypes);
				}
				case PrimitiveTemplatedMethod: {
					llvm::Type* const memberTypes[] = {
						TypeGenerator(module).getPtrType(),
						getBasicPrimitiveType(module, PrimitiveTemplatedMethodFunctionPtr)
					};
					return TypeGenerator(module).getStructType(memberTypes);
				}
				case PrimitiveInterfaceMethod: {
					return interfaceMethodType(module).second;
				}
				case PrimitiveStaticInterfaceMethod: {
					return staticInterfaceMethodType(module).second;
				}
				case PrimitiveTypename:
					return typeInfoType(module).second;
				default:
					llvm_unreachable("Unrecognised primitive type.");
			}
		}
		
		llvm_abi::Type* getBasicPrimitiveABIType(Module& module, const PrimitiveID id) {
			auto& abiContext = module.abiContext();
			
			switch (id) {
				case PrimitiveVoid:
					// TODO: use a void type?
					return llvm_abi::Type::Struct(abiContext, {});
				case PrimitiveNull:
				case PrimitivePtr:
				case PrimitiveFunctionPtr:
				case PrimitiveMethodFunctionPtr:
				case PrimitiveVarArgFunctionPtr:
					return llvm_abi::Type::Pointer(abiContext);
				case PrimitiveTemplatedFunctionPtr:
				case PrimitiveTemplatedMethodFunctionPtr: {
					std::vector<llvm_abi::Type*> types;
					types.reserve(2);
					types.push_back(llvm_abi::Type::Pointer(abiContext));
					types.push_back(templateGeneratorType(module).first);
					return llvm_abi::Type::AutoStruct(abiContext, types);
				}
				case PrimitiveMethod: {
					std::vector<llvm_abi::Type*> types;
					types.reserve(2);
					types.push_back(llvm_abi::Type::Pointer(abiContext));
					types.push_back(getBasicPrimitiveABIType(module, PrimitiveMethodFunctionPtr));
					return llvm_abi::Type::AutoStruct(abiContext, types);
				}
				case PrimitiveTemplatedMethod: {
					std::vector<llvm_abi::Type*> types;
					types.reserve(2);
					types.push_back(llvm_abi::Type::Pointer(abiContext));
					types.push_back(getBasicPrimitiveABIType(module, PrimitiveTemplatedMethodFunctionPtr));
					return llvm_abi::Type::AutoStruct(abiContext, types);
				}
				case PrimitiveInterfaceMethod: {
					return interfaceMethodType(module).first;
				}
				case PrimitiveStaticInterfaceMethod: {
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
					return llvm_abi::Type::Integer(abiContext, primitiveABIIntegerKind(id));
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
		
		llvm_abi::Type* getPrimitiveABIType(Module& module, const SEM::Type* const type) {
			assert(TypeInfo(module).isSizeKnownInThisModule(type));
			
			const auto& primitive = module.getPrimitive(*(type->getObjectType()));
			return primitive.getABIType(module,
			                            module.abiContext(),
			                            arrayRef(type->templateArguments()));
		}
		
	}
	
}

