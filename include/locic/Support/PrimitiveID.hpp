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
		PrimitiveUnichar,
		
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
		PrimitiveAbstractTypename,
		PrimitiveTypename,
		
		PrimitiveRange,
		PrimitiveRangeIncl,
		PrimitiveReverseRange,
		PrimitiveReverseRangeIncl,
		
		PrimitiveStaticArray
	};
	
#define NAME_CASE(id, prefix, suffix) \
	case id ## 0: \
		return prefix "0_" suffix; \
	case id ## 1: \
		return prefix "1_" suffix; \
	case id ## 2: \
		return prefix "2_" suffix; \
	case id ## 3: \
		return prefix "3_" suffix; \
	case id ## 4: \
		return prefix "4_" suffix; \
	case id ## 5: \
		return prefix "5_" suffix; \
	case id ## 6: \
		return prefix "6_" suffix; \
	case id ## 7: \
		return prefix "7_" suffix; \
	case id ## 8: \
		return prefix "8_" suffix
	
#define PRIMITIVE_STRING_CASE(primitiveID, prefix) \
	switch (primitiveID) { \
		case PrimitiveVoid: \
			return prefix "void_t"; \
		case PrimitiveNull: \
			return prefix "null_t"; \
		case PrimitiveBool: \
			return prefix "bool_t"; \
		case PrimitiveCompareResult: \
			return prefix "compare_result_t"; \
		case PrimitiveUnichar: \
			return prefix "unichar_t"; \
		NAME_CASE(PrimitiveFunctionPtr, prefix "function", "ptr_t"); \
		NAME_CASE(PrimitiveMethodFunctionPtr, prefix "methodfunction", "ptr_t"); \
		NAME_CASE(PrimitiveTemplatedFunctionPtr, prefix "templatedfunction", "ptr_t"); \
		NAME_CASE(PrimitiveTemplatedMethodFunctionPtr, prefix "templatedmethodfunction", "ptr_t"); \
		NAME_CASE(PrimitiveVarArgFunctionPtr, prefix "varargfunction", "ptr_t"); \
		NAME_CASE(PrimitiveMethod, prefix "method", "t"); \
		NAME_CASE(PrimitiveTemplatedMethod, prefix "templatedmethod", "t"); \
		NAME_CASE(PrimitiveInterfaceMethod, prefix "interfacemethod", "t"); \
		NAME_CASE(PrimitiveStaticInterfaceMethod, prefix "staticinterfacemethod", "t"); \
		case PrimitiveInt8: \
			return prefix "int8_t"; \
		case PrimitiveUInt8: \
			return prefix "uint8_t"; \
		case PrimitiveInt16: \
			return prefix "int16_t"; \
		case PrimitiveUInt16: \
			return prefix "uint16_t"; \
		case PrimitiveInt32: \
			return prefix "int32_t"; \
		case PrimitiveUInt32: \
			return prefix "uint32_t"; \
		case PrimitiveInt64: \
			return prefix "int64_t"; \
		case PrimitiveUInt64: \
			return prefix "uint64_t"; \
		case PrimitiveByte: \
			return prefix "byte_t"; \
		case PrimitiveUByte: \
			return prefix "ubyte_t"; \
		case PrimitiveShort: \
			return prefix "short_t"; \
		case PrimitiveUShort: \
			return prefix "ushort_t"; \
		case PrimitiveInt: \
			return prefix "int_t"; \
		case PrimitiveUInt: \
			return prefix "uint_t"; \
		case PrimitiveLong: \
			return prefix "long_t"; \
		case PrimitiveULong: \
			return prefix "ulong_t"; \
		case PrimitiveLongLong: \
			return prefix "longlong_t"; \
		case PrimitiveULongLong: \
			return prefix "ulonglong_t"; \
		case PrimitiveSize: \
			return prefix "size_t"; \
		case PrimitiveSSize: \
			return prefix "ssize_t"; \
		case PrimitivePtrDiff: \
			return prefix "ptrdiff_t"; \
		case PrimitiveFloat: \
			return prefix "float_t"; \
		case PrimitiveDouble: \
			return prefix "double_t"; \
		case PrimitiveLongDouble: \
			return prefix "longdouble_t"; \
		case PrimitiveRef: \
			return prefix "ref_t"; \
		case PrimitivePtr: \
			return prefix "ptr_t"; \
		case PrimitivePtrLval: \
			return prefix "ptr_lval_t"; \
		case PrimitiveValueLval: \
			return prefix "value_lval_t"; \
		case PrimitiveAbstractTypename: \
			return prefix "abstracttypename_t"; \
		case PrimitiveTypename: \
			return prefix "typename_t"; \
		case PrimitiveRange: \
			return prefix "range_t"; \
		case PrimitiveRangeIncl: \
			return prefix "range_incl_t"; \
		case PrimitiveReverseRange: \
			return prefix "reverse_range_t"; \
		case PrimitiveReverseRangeIncl: \
			return prefix "reverse_range_incl_t"; \
		case PrimitiveStaticArray: \
			return prefix "static_array_t"; \
	} \
	locic_unreachable("Unknown primitive ID.");
	
	constexpr size_t PRIMITIVE_COUNT = PrimitiveStaticArray + 1;
	
	class PrimitiveID {
	public:
		PrimitiveID() = default;
		
		PrimitiveID(const PrimitiveIDEnum value)
		: value_(value) { }
		
		static PrimitiveID Callable(const PrimitiveIDEnum baseValue,
		                            const size_t argumentCount);
		static PrimitiveID FunctionPtr(const size_t argumentCount);
		static PrimitiveID MethodFunctionPtr(const size_t argumentCount);
		static PrimitiveID TemplatedFunctionPtr(const size_t argumentCount);
		static PrimitiveID TemplatedMethodFunctionPtr(const size_t argumentCount);
		static PrimitiveID VarArgFunctionPtr(const size_t argumentCount);
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
