#ifndef LOCIC_SUPPORT_PRIMITIVEID_HPP
#define LOCIC_SUPPORT_PRIMITIVEID_HPP

#include <stdexcept>
#include <string>

namespace locic {
	
	// TODO: implement variadic templates so this can be removed.
#define REPEAT_CALLABLE_ID(id) \
	id ## 0, \
	id ## 1, \
	id ## 2, \
	id ## 3, \
	id ## 4, \
	id ## 5, \
	id ## 6, \
	id ## 7, \
	id ## 8
	
#define CASE_CALLABLE_ID(id) \
	case id ## 0: \
	case id ## 1: \
	case id ## 2: \
	case id ## 3: \
	case id ## 4: \
	case id ## 5: \
	case id ## 6: \
	case id ## 7: \
	case id ## 8
	
	enum PrimitiveIDEnum {
		PrimitiveVoid = 0,
		PrimitiveNull,
		PrimitiveBool,
		PrimitiveCompareResult,
		
		REPEAT_CALLABLE_ID(PrimitiveFunctionPtr),
		REPEAT_CALLABLE_ID(PrimitiveMethodFunctionPtr),
		REPEAT_CALLABLE_ID(PrimitiveTemplatedFunctionPtr),
		REPEAT_CALLABLE_ID(PrimitiveTemplatedMethodFunctionPtr),
		REPEAT_CALLABLE_ID(PrimitiveVarArgFunctionPtr),
		
		REPEAT_CALLABLE_ID(PrimitiveMethod),
		REPEAT_CALLABLE_ID(PrimitiveTemplatedMethod),
		REPEAT_CALLABLE_ID(PrimitiveInterfaceMethod),
		REPEAT_CALLABLE_ID(PrimitiveStaticInterfaceMethod),
		
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
		
		PrimitiveCount,
		PrimitiveCountIncl,
		PrimitiveRange,
		PrimitiveRangeIncl,
		PrimitiveReverseRange,
		PrimitiveReverseRangeIncl,
		
		PrimitiveStaticArray
	};
	
	constexpr size_t PRIMITIVE_COUNT = PrimitiveStaticArray + 1;
	
	class PrimitiveID {
	public:
		PrimitiveID(const PrimitiveIDEnum value)
		: value_(value) { }
		
		static PrimitiveID Callable(const PrimitiveIDEnum baseValue,
		                            const size_t argumentCount);
		static PrimitiveID FunctionPtr(const size_t argumentCount);
		static PrimitiveID MethodFunctionPtr(const size_t argumentCount);
		static PrimitiveID TemplatedFunctionPtr(const size_t argumentCount);
		static PrimitiveID TemplatedMethodFunctionPtr(const size_t argumentCount);
		static PrimitiveID VarArgFunctionnPtr(const size_t argumentCount);
		static PrimitiveID Method(const size_t argumentCount);
		static PrimitiveID TemplatedMethod(const size_t argumentCount);
		static PrimitiveID InterfaceMethod(const size_t argumentCount);
		static PrimitiveID StaticInterfaceMethod(const size_t argumentCount);
		
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
		bool isFixedSizeInteger() const;
		bool isFloat() const;
		bool isCallable() const;
		
		PrimitiveID asUnsigned() const;
		
		size_t getIntegerMinByteSize() const;
		
		PrimitiveID baseCallableID() const;
		
		bool isSubsetOf(const PrimitiveID other) const;
		bool isSupersetOf(const PrimitiveID other) const;
		
		const char* toCString() const;
		
		std::string toString() const;
		
	private:
		PrimitiveIDEnum value_;
		
	};
	
}

#endif
