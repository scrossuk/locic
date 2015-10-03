#ifndef LOCIC_SUPPORT_PRIMITIVEID_HPP
#define LOCIC_SUPPORT_PRIMITIVEID_HPP

#include <stdexcept>
#include <string>

namespace locic {
	
	enum PrimitiveIDEnum {
		PrimitiveVoid = 0,
		PrimitiveNull,
		PrimitiveBool,
		PrimitiveCompareResult,
		
		PrimitiveFunctionPtr,
		PrimitiveMethodFunctionPtr,
		PrimitiveTemplatedFunctionPtr,
		PrimitiveTemplatedMethodFunctionPtr,
		PrimitiveVarArgFunctionPtr,
		
		PrimitiveMethod,
		PrimitiveTemplatedMethod,
		PrimitiveInterfaceMethod,
		PrimitiveStaticInterfaceMethod,
		
		PrimitiveInt8,
		PrimitiveUInt8,
		PrimitiveInt16,
		PrimitiveUInt16,
		PrimitiveInt32,
		PrimitiveUInt32,
		PrimitiveInt64,
		PrimitiveUInt64,
		
		PrimitiveByte,
		PrimitiveUByte,
		PrimitiveShort,
		PrimitiveUShort,
		PrimitiveInt,
		PrimitiveUInt,
		PrimitiveLong,
		PrimitiveULong,
		PrimitiveLongLong,
		PrimitiveULongLong,
		
		PrimitiveSize,
		PrimitiveSSize,
		
		PrimitivePtrDiff,
		
		PrimitiveFloat,
		PrimitiveDouble,
		PrimitiveLongDouble,
		PrimitiveRef,
		PrimitivePtr,
		PrimitivePtrLval,
		PrimitiveValueLval,
		PrimitiveFinalLval,
		PrimitiveTypename,
		
		PrimitiveStaticArray
	};
	
	constexpr size_t PRIMITIVE_COUNT = PrimitiveStaticArray + 1;
	
	class PrimitiveID {
	public:
		PrimitiveID(const PrimitiveIDEnum value)
		: value_(value) { }
		
		bool operator==(const PrimitiveID& other) const {
			return value_ == other.value_;
		}
		
		bool operator!=(const PrimitiveID& other) const {
			return value_ != other.value_;
		}
		
		bool operator==(const PrimitiveIDEnum& other) const {
			return value_ == other;
		}
		
		bool operator!=(const PrimitiveIDEnum& other) const {
			return value_ != other;
		}
		
		operator PrimitiveIDEnum() const {
			return value_;
		}
		
		bool isLval() const;
		bool isInteger() const;
		bool isSignedInteger() const;
		bool isUnsignedInteger() const;
		bool isFloat() const;
		bool isCallable() const;
		
		std::string getName(const size_t count) const;
		
		const char* toCString() const;
		
		std::string toString() const;
		
	private:
		PrimitiveIDEnum value_;
		
	};
	
}

#endif
