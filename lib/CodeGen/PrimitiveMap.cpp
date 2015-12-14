#include <memory>

#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/PrimitiveMap.hpp>
#include <locic/CodeGen/Primitives/BoolPrimitive.hpp>
#include <locic/CodeGen/Primitives/CompareResultPrimitive.hpp>
#include <locic/CodeGen/Primitives/FinalLvalPrimitive.hpp>
#include <locic/CodeGen/Primitives/FloatPrimitive.hpp>
#include <locic/CodeGen/Primitives/FunctionPtrPrimitive.hpp>
#include <locic/CodeGen/Primitives/NullPrimitive.hpp>
#include <locic/CodeGen/Primitives/PtrLvalPrimitive.hpp>
#include <locic/CodeGen/Primitives/PtrPrimitive.hpp>
#include <locic/CodeGen/Primitives/RangePrimitive.hpp>
#include <locic/CodeGen/Primitives/RefPrimitive.hpp>
#include <locic/CodeGen/Primitives/SignedIntegerPrimitive.hpp>
#include <locic/CodeGen/Primitives/StaticArrayPrimitive.hpp>
#include <locic/CodeGen/Primitives/TypenamePrimitive.hpp>
#include <locic/CodeGen/Primitives/UnsignedIntegerPrimitive.hpp>
#include <locic/CodeGen/Primitives/ValueLvalPrimitive.hpp>
#include <locic/CodeGen/Primitives/VoidPrimitive.hpp>

#include <locic/SEM/TypeInstance.hpp>

#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		Primitive* createPrimitive(const SEM::TypeInstance& typeInstance) {
			const auto primitiveID = typeInstance.primitiveID();
			switch (primitiveID) {
				case PrimitiveVoid: {
					return new VoidPrimitive(typeInstance);
				}
				case PrimitiveNull: {
					return new NullPrimitive(typeInstance);
				}
				case PrimitiveBool: {
					return new BoolPrimitive(typeInstance);
				}
				case PrimitiveCompareResult: {
					return new CompareResultPrimitive(typeInstance);
				}
				CASE_CALLABLE_ID(PrimitiveFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveMethodFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveTemplatedFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveTemplatedMethodFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveVarArgFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveMethod):
				CASE_CALLABLE_ID(PrimitiveTemplatedMethod):
				CASE_CALLABLE_ID(PrimitiveInterfaceMethod):
				CASE_CALLABLE_ID(PrimitiveStaticInterfaceMethod): {
					return new FunctionPtrPrimitive(typeInstance);
				}
				case PrimitiveInt8:
				case PrimitiveInt16:
				case PrimitiveInt32:
				case PrimitiveInt64:
				case PrimitiveByte:
				case PrimitiveShort:
				case PrimitiveInt:
				case PrimitiveLong:
				case PrimitiveLongLong:
				case PrimitiveSSize:
				case PrimitivePtrDiff: {
					return new SignedIntegerPrimitive(typeInstance);
				}
				case PrimitiveUInt8:
				case PrimitiveUInt16:
				case PrimitiveUInt32:
				case PrimitiveUInt64:
				case PrimitiveUByte:
				case PrimitiveUShort:
				case PrimitiveUInt:
				case PrimitiveULong:
				case PrimitiveULongLong:
				case PrimitiveSize: {
					return new UnsignedIntegerPrimitive(typeInstance);
				}
				case PrimitiveFloat:
				case PrimitiveDouble:
				case PrimitiveLongDouble: {
					return new FloatPrimitive(typeInstance);
				}
				case PrimitiveRef: {
					return new RefPrimitive(typeInstance);
				}
				case PrimitivePtr: {
					return new PtrPrimitive(typeInstance);
				}
				case PrimitivePtrLval: {
					return new PtrLvalPrimitive(typeInstance);
				}
				case PrimitiveValueLval: {
					return new ValueLvalPrimitive(typeInstance);
				}
				case PrimitiveFinalLval: {
					return new FinalLvalPrimitive(typeInstance);
				}
				case PrimitiveTypename: {
					return new TypenamePrimitive(typeInstance);
				}
				case PrimitiveRange:
				case PrimitiveRangeIncl:
				case PrimitiveReverseRange:
				case PrimitiveReverseRangeIncl: {
					return new RangePrimitive(typeInstance);
				}
				case PrimitiveStaticArray: {
					return new StaticArrayPrimitive(typeInstance);
				}
			}
			
			llvm_unreachable("Invalid PrimitiveID.");
		}
		
		PrimitiveMap::PrimitiveMap() { }
		
		PrimitiveMap::~PrimitiveMap() { }
		
		const Primitive&
		PrimitiveMap::getPrimitive(const SEM::TypeInstance& typeInstance) const {
			const auto iterator = primitives_.find(&typeInstance);
			if (iterator != primitives_.end()) {
				return *(iterator->second);
			}
			std::unique_ptr<Primitive> primitive(createPrimitive(typeInstance));
			const auto primitivePtr = primitive.get();
			primitives_.insert(std::make_pair(&typeInstance, std::move(primitive)));
			return *primitivePtr;
		}
		
	}
	
}
