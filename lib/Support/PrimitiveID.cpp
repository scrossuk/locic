#include <cassert>
#include <string>

#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
	PrimitiveID PrimitiveID::Callable(const PrimitiveIDEnum baseValue,
	                                  const size_t argumentCount) {
		assert(0 <= argumentCount && argumentCount <= 8);
		return PrimitiveID(static_cast<PrimitiveIDEnum>(baseValue + argumentCount));
	}
	
	PrimitiveID PrimitiveID::FunctionPtr(const size_t argumentCount) {
		return PrimitiveID::Callable(PrimitiveFunctionPtr0,
		                             argumentCount);
	}
	
	PrimitiveID PrimitiveID::MethodFunctionPtr(const size_t argumentCount) {
		return PrimitiveID::Callable(PrimitiveMethodFunctionPtr0,
		                             argumentCount);
	}
	
	PrimitiveID PrimitiveID::TemplatedFunctionPtr(const size_t argumentCount) {
		return PrimitiveID::Callable(PrimitiveTemplatedFunctionPtr0,
		                             argumentCount);
	}
	
	PrimitiveID PrimitiveID::TemplatedMethodFunctionPtr(const size_t argumentCount) {
		return PrimitiveID::Callable(PrimitiveTemplatedMethodFunctionPtr0,
		                             argumentCount);
	}
	
	PrimitiveID PrimitiveID::VarArgFunctionPtr(const size_t argumentCount) {
		return PrimitiveID::Callable(PrimitiveVarArgFunctionPtr0,
		                             argumentCount);
	}
	
	PrimitiveID PrimitiveID::Method(const size_t argumentCount) {
		return PrimitiveID::Callable(PrimitiveMethod0,
		                             argumentCount);
	}
	
	PrimitiveID PrimitiveID::TemplatedMethod(const size_t argumentCount) {
		return PrimitiveID::Callable(PrimitiveTemplatedMethod0,
		                             argumentCount);
	}
	
	PrimitiveID PrimitiveID::InterfaceMethod(const size_t argumentCount) {
		return PrimitiveID::Callable(PrimitiveInterfaceMethod0,
		                             argumentCount);
	}
	
	PrimitiveID PrimitiveID::StaticInterfaceMethod(const size_t argumentCount) {
		return PrimitiveID::Callable(PrimitiveStaticInterfaceMethod0,
		                             argumentCount);
	}
	
	bool PrimitiveID::isLval() const {
		switch (value_) {
			case PrimitivePtrLval:
			case PrimitiveValueLval:
			case PrimitiveFinalLval:
				return true;
			default:
				return false;
		}
	}
	
	bool PrimitiveID::isInteger() const {
		return isSignedInteger() || isUnsignedInteger();
	}
	
	bool PrimitiveID::isSignedInteger() const {
		switch (value_) {
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
			case PrimitivePtrDiff:
				return true;
			default:
				return false;
		}
	}
	
	bool PrimitiveID::isUnsignedInteger() const {
		switch (value_) {
			case PrimitiveUInt8:
			case PrimitiveUInt16:
			case PrimitiveUInt32:
			case PrimitiveUInt64:
			case PrimitiveUByte:
			case PrimitiveUShort:
			case PrimitiveUInt:
			case PrimitiveULong:
			case PrimitiveULongLong:
			case PrimitiveSize:
				return true;
			default:
				return false;
		}
	}
	
	bool PrimitiveID::isFixedSizeInteger() const {
		switch (value_) {
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
				return true;
			default:
				return false;
		}
	}
	
	bool PrimitiveID::isFloat() const {
		switch (value_) {
			case PrimitiveFloat:
			case PrimitiveDouble:
			case PrimitiveLongDouble:
				return true;
			default:
				return false;
		}
	}
	
	bool PrimitiveID::isCallable() const {
		switch (value_) {
			CASE_CALLABLE_ID(PrimitiveFunctionPtr):
			CASE_CALLABLE_ID(PrimitiveMethodFunctionPtr):
			CASE_CALLABLE_ID(PrimitiveTemplatedFunctionPtr):
			CASE_CALLABLE_ID(PrimitiveTemplatedMethodFunctionPtr):
			CASE_CALLABLE_ID(PrimitiveVarArgFunctionPtr):
			CASE_CALLABLE_ID(PrimitiveMethod):
			CASE_CALLABLE_ID(PrimitiveTemplatedMethod):
			CASE_CALLABLE_ID(PrimitiveInterfaceMethod):
			CASE_CALLABLE_ID(PrimitiveStaticInterfaceMethod):
				return true;
			default:
				return false;
		}
	}
	
	PrimitiveID PrimitiveID::baseCallableID() const {
		switch (value_) {
			CASE_CALLABLE_ID(PrimitiveFunctionPtr):
				return PrimitiveFunctionPtr0;
			
			CASE_CALLABLE_ID(PrimitiveMethodFunctionPtr):
				return PrimitiveMethodFunctionPtr0;
			
			CASE_CALLABLE_ID(PrimitiveTemplatedFunctionPtr):
				return PrimitiveTemplatedFunctionPtr0;
			
			CASE_CALLABLE_ID(PrimitiveTemplatedMethodFunctionPtr):
				return PrimitiveTemplatedMethodFunctionPtr0;
			
			CASE_CALLABLE_ID(PrimitiveVarArgFunctionPtr):
				return PrimitiveVarArgFunctionPtr0;
			
			CASE_CALLABLE_ID(PrimitiveMethod):
				return PrimitiveMethod0;
			
			CASE_CALLABLE_ID(PrimitiveTemplatedMethod):
				return PrimitiveTemplatedMethod0;
			
			CASE_CALLABLE_ID(PrimitiveInterfaceMethod):
				return PrimitiveInterfaceMethod0;
			
			CASE_CALLABLE_ID(PrimitiveStaticInterfaceMethod):
				return PrimitiveStaticInterfaceMethod0;
			
			default:
				return *this;
		}
	}
	
	PrimitiveID PrimitiveID::asUnsigned() const {
		assert(isInteger());
		
		if (isUnsignedInteger()) {
			return *this;
		}
		
		switch (value_) {
			case PrimitiveInt8:
				return PrimitiveUInt8;
			case PrimitiveInt16:
				return PrimitiveUInt16;
			case PrimitiveInt32:
				return PrimitiveUInt32;
			case PrimitiveInt64:
				return PrimitiveUInt64;
			case PrimitiveByte:
				return PrimitiveUByte;
			case PrimitiveShort:
				return PrimitiveUShort;
			case PrimitiveInt:
				return PrimitiveUInt;
			case PrimitiveLong:
				return PrimitiveULong;
			case PrimitiveLongLong:
				return PrimitiveULongLong;
			case PrimitiveSSize:
				return PrimitiveSize;
			case PrimitivePtrDiff:
				// TODO?
				return PrimitiveSize;
			default:
				locic_unreachable("Unknown integer type.");
		}
	}
	
	size_t PrimitiveID::getIntegerMinByteSize() const {
		switch (value_) {
			case PrimitiveInt8:
			case PrimitiveUInt8:
				return 1;
			case PrimitiveInt16:
			case PrimitiveUInt16:
				return 2;
			case PrimitiveInt32:
			case PrimitiveUInt32:
				return 4;
			case PrimitiveInt64:
			case PrimitiveUInt64:
				return 8;
			case PrimitiveByte:
			case PrimitiveUByte:
				return 1;
			case PrimitiveShort:
			case PrimitiveUShort:
			case PrimitiveInt:
			case PrimitiveUInt:
				return 2;
			case PrimitiveLong:
			case PrimitiveULong:
				return 4;
			case PrimitiveLongLong:
			case PrimitiveULongLong:
				return 8;
			case PrimitiveSize:
			case PrimitiveSSize:
				return 2;
			case PrimitivePtrDiff:
				return 2;
			default:
				locic_unreachable("Not an integer type.");
		}
	}
	
	bool PrimitiveID::isSubsetOf(const PrimitiveID other) const {
		if (isInteger() && other.isInteger()) {
			if (isSignedInteger() && other.isUnsignedInteger()) {
				// Can't represent negative values with an
				// unsigned integer.
				return false;
			}
			
			if (!isFixedSizeInteger() && other.isFixedSizeInteger()) {
				// Unknown sized integers could be larger than
				// fixed sized integers (e.g. on a special
				// architecture).
				return false;
			}
			
			const auto minByteSize = getIntegerMinByteSize();
			const auto otherMinByteSize = other.getIntegerMinByteSize();
			
			if (isUnsignedInteger() && other.isSignedInteger()) {
				// Signed integer needs to be larger since it
				// must also have a sign bit.
				return minByteSize < otherMinByteSize;
			} else {
				return minByteSize <= otherMinByteSize;
			}
		} else if (isFloat() && other.isFloat()) {
			switch (value_) {
				case PrimitiveFloat:
					return true;
				case PrimitiveDouble:
					return other != PrimitiveFloat;
				case PrimitiveLongDouble:
					return other == PrimitiveLongDouble;
				default:
					locic_unreachable("Unknown float type.");
			}
		} else {
			return false;
		}
	}
	
	bool PrimitiveID::isSupersetOf(const PrimitiveID other) const {
		return other.isSubsetOf(*this);
	}
	
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
	
	const char* PrimitiveID::toCString() const {
		switch (value_) {
			case PrimitiveVoid:
				return "void_t";
			case PrimitiveNull:
				return "null_t";
			case PrimitiveBool:
				return "bool_t";
			case PrimitiveCompareResult:
				return "compare_result_t";
			case PrimitiveUnichar:
				return "unichar_t";
			NAME_CASE(PrimitiveFunctionPtr, "function", "ptr_t");
			NAME_CASE(PrimitiveMethodFunctionPtr, "methodfunction", "ptr_t");
			NAME_CASE(PrimitiveTemplatedFunctionPtr, "templatedfunction", "ptr_t");
			NAME_CASE(PrimitiveTemplatedMethodFunctionPtr, "templatedmethodfunction", "ptr_t");
			NAME_CASE(PrimitiveVarArgFunctionPtr, "varargfunction", "ptr_t");
			NAME_CASE(PrimitiveMethod, "method", "t");
			NAME_CASE(PrimitiveTemplatedMethod, "templatedmethod", "t");
			NAME_CASE(PrimitiveInterfaceMethod, "interfacemethod", "t");
			NAME_CASE(PrimitiveStaticInterfaceMethod, "staticinterfacemethod", "t");
			case PrimitiveInt8:
				return "int8_t";
			case PrimitiveUInt8:
				return "uint8_t";
			case PrimitiveInt16:
				return "int16_t";
			case PrimitiveUInt16:
				return "uint16_t";
			case PrimitiveInt32:
				return "int32_t";
			case PrimitiveUInt32:
				return "uint32_t";
			case PrimitiveInt64:
				return "int64_t";
			case PrimitiveUInt64:
				return "uint64_t";
			case PrimitiveByte:
				return "byte_t";
			case PrimitiveUByte:
				return "ubyte_t";
			case PrimitiveShort:
				return "short_t";
			case PrimitiveUShort:
				return "ushort_t";
			case PrimitiveInt:
				return "int_t";
			case PrimitiveUInt:
				return "uint_t";
			case PrimitiveLong:
				return "long_t";
			case PrimitiveULong:
				return "ulong_t";
			case PrimitiveLongLong:
				return "longlong_t";
			case PrimitiveULongLong:
				return "ulonglong_t";
			case PrimitiveSize:
				return "size_t";
			case PrimitiveSSize:
				return "ssize_t";
			case PrimitivePtrDiff:
				return "ptrdiff_t";
			case PrimitiveFloat:
				return "float_t";
			case PrimitiveDouble:
				return "double_t";
			case PrimitiveLongDouble:
				return "longdouble_t";
			case PrimitiveRef:
				return "ref_t";
			case PrimitivePtr:
				return "ptr_t";
			case PrimitivePtrLval:
				return "ptr_lval_t";
			case PrimitiveValueLval:
				return "value_lval_t";
			case PrimitiveFinalLval:
				return "final_lval_t";
			case PrimitiveTypename:
				return "typename_t";
			case PrimitiveRange:
				return "range_t";
			case PrimitiveRangeIncl:
				return "range_incl_t";
			case PrimitiveReverseRange:
				return "reverse_range_t";
			case PrimitiveReverseRangeIncl:
				return "reverse_range_incl_t";
			case PrimitiveStaticArray:
				return "static_array_t";
		}
		
		locic_unreachable("Unknown primitive ID.");
	}
	
	std::string PrimitiveID::toString() const {
		return toCString();
	}
	
}
