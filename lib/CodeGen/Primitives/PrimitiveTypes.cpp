#include <assert.h>

#include <stdexcept>

#include <locic/Support/String.hpp>
#include <locic/CodeGen/ArgInfo.hpp>
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
	
		llvm::PointerType* getPrimitivePointerType(Module& module, const SEM::Type* const type) {
			const auto& name = type->getObjectType()->name().last();
			const auto kind = module.primitiveKind(name);
			
			switch (kind) {
				case PrimitiveValueLval:
				case PrimitiveFinalLval: {
					return genPointerType(module, type->templateArguments().front().typeRefType());
				}
				default: {
					const auto pointerType = getPrimitiveType(module, type);
					if (pointerType->isVoidTy()) {
						// LLVM doesn't support 'void *' => use 'int8_t *' instead.
						return TypeGenerator(module).getI8PtrType();
					} else {
						return pointerType->getPointerTo();
					}
				}
			}
		}
		
		llvm::Type* getFunctionPointerType(Module& module, const SEM::FunctionType functionType) {
			const auto functionPtrType = genFunctionType(module, functionType)->getPointerTo();
			if (functionType.attributes().isTemplated()) {
				llvm::Type* const memberTypes[] = {
					functionPtrType,
					templateGeneratorType(module).second
				};
				return TypeGenerator(module).getStructType(memberTypes);
			} else {
				return functionPtrType;
			}
		}
		
		llvm::Type* getPrimitiveType(Module& module, const SEM::Type* const type) {
			const auto& name = type->getObjectType()->name().last();
			const auto kind = module.primitiveKind(name);
			
			switch (kind) {
				case PrimitiveRef: {
					const auto argType = type->templateArguments().front().typeRefType();
					if (argType->isTemplateVar() && argType->getTemplateVar()->isVirtual()) {
						// Unknown whether the argument type is virtual, so use an opaque struct type.
						const auto iterator = module.typeInstanceMap().find(type->getObjectType());
						if (iterator != module.typeInstanceMap().end()) {
							return iterator->second;
						}
						
						const auto structType = TypeGenerator(module).getForwardDeclaredStructType(module.getCString("ref_t"));
						
						module.typeInstanceMap().insert(std::make_pair(type->getObjectType(), structType));
						return structType;
					} else if (type->templateArguments().front().typeRefType()->isInterface()) {
						// Argument type is definitely virtual.
						return interfaceStructType(module).second;
					} else {
						// Argument type is definitely not virtual.
						return genPointerType(module, type->templateArguments().front().typeRefType());
					}
				}
				case PrimitivePtr:
				case PrimitivePtrLval:
					return genPointerType(module, type->templateArguments().front().typeRefType());
				case PrimitiveFunctionPtr:
				case PrimitiveMethodFunctionPtr:
				case PrimitiveVarArgFunctionPtr:
				case PrimitiveTemplatedFunctionPtr:
				case PrimitiveTemplatedMethodFunctionPtr: {
					return getFunctionPointerType(module, type->asFunctionType());
				}
				case PrimitiveMethod:
				case PrimitiveTemplatedMethod: {
					llvm::Type* const memberTypes[] = {
						TypeGenerator(module).getI8PtrType(),
						getFunctionPointerType(module, type->asFunctionType())
					};
					return TypeGenerator(module).getStructType(memberTypes);
				}
				case PrimitiveInterfaceMethod: {
					return interfaceMethodType(module).second;
				}
				case PrimitiveStaticInterfaceMethod: {
					return staticInterfaceMethodType(module).second;
				}
				case PrimitiveValueLval:
				case PrimitiveFinalLval: {
					return genType(module, type->templateArguments().front().typeRefType());
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
				case PrimitiveFunctionPtr:
				case PrimitiveMethodFunctionPtr:
				case PrimitiveVarArgFunctionPtr:
				case PrimitivePtrLval:
					return TypeGenerator(module).getI8PtrType();
				case PrimitiveTemplatedFunctionPtr:
				case PrimitiveTemplatedMethodFunctionPtr: {
					llvm::Type* const memberTypes[] = {
						TypeGenerator(module).getI8PtrType(),
						templateGeneratorType(module).second
					};
					return TypeGenerator(module).getStructType(memberTypes);
				}
				case PrimitiveMethod: {
					llvm::Type* const memberTypes[] = {
						TypeGenerator(module).getI8PtrType(),
						getBasicPrimitiveType(module, PrimitiveMethodFunctionPtr)
					};
					return TypeGenerator(module).getStructType(memberTypes);
				}
				case PrimitiveTemplatedMethod: {
					llvm::Type* const memberTypes[] = {
						TypeGenerator(module).getI8PtrType(),
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
		
		llvm::Type* getNamedPrimitiveType(Module& module, const String& name) {
			return getBasicPrimitiveType(module, module.primitiveKind(name));
		}
		
		llvm_abi::Type* getBasicPrimitiveABIType(Module& module, PrimitiveKind kind) {
			auto& abiContext = module.abiContext();
			
			switch (kind) {
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
		
		llvm_abi::Type* getNamedPrimitiveABIType(Module& module, const String& name) {
			return getBasicPrimitiveABIType(module, module.primitiveKind(name));
		}
		
		llvm_abi::Type* getPrimitiveABIType(Module& module, const SEM::Type* const type) {
			assert(isTypeSizeKnownInThisModule(module, type));
			
			const auto typeInstance = type->getObjectType();
			const auto& name = typeInstance->name().last();
			
			auto& abiContext = module.abiContext();
			const auto kind = module.primitiveKind(name);
			
			switch (kind) {
				case PrimitiveRef: {
					if (type->templateArguments().front().typeRefType()->isInterface()) {
						return interfaceStructType(module).first;
					} else {
						return llvm_abi::Type::Pointer(abiContext);
					}
				}
				case PrimitivePtrLval:
					return llvm_abi::Type::Pointer(abiContext);
				case PrimitiveValueLval:
				case PrimitiveFinalLval:
					return genABIType(module, type->templateArguments().front().typeRefType());
				case PrimitiveTypename:
					return typeInfoType(module).first;
				default:
					return getBasicPrimitiveABIType(module, kind);
			}
		}
		
	}
	
}

