#include <cassert>
#include <string>

#include <locic/Support/MakeString.hpp>
#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
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
			case PrimitiveFunctionPtr:
			case PrimitiveMethodFunctionPtr:
			case PrimitiveTemplatedFunctionPtr:
			case PrimitiveTemplatedMethodFunctionPtr:
			case PrimitiveVarArgFunctionPtr:
			case PrimitiveMethod:
			case PrimitiveTemplatedMethod:
			case PrimitiveInterfaceMethod:
			case PrimitiveStaticInterfaceMethod:
				return true;
			default:
				return false;
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
				throw std::logic_error("Unknown integer type.");
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
				throw std::logic_error("Not an integer type.");
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
					throw std::logic_error("Unknown float type.");
			}
		} else {
			return false;
		}
	}
	
	bool PrimitiveID::isSupersetOf(const PrimitiveID other) const {
		return other.isSubsetOf(*this);
	}
	
	std::string PrimitiveID::getName(const size_t count) const {
		switch (value_) {
			case PrimitiveFunctionPtr:
				return makeString("function%llu_ptr_t",
					(unsigned long long) count);
			case PrimitiveMethodFunctionPtr:
				return makeString("methodfunction%llu_ptr_t",
					(unsigned long long) count);
			case PrimitiveTemplatedFunctionPtr:
				return makeString("templatedfunction%llu_ptr_t",
					(unsigned long long) count);
			case PrimitiveTemplatedMethodFunctionPtr:
				return makeString("templatedmethodfunction%llu_ptr_t",
					(unsigned long long) count);
			case PrimitiveVarArgFunctionPtr:
				return makeString("varargfunction%llu_ptr_t",
					(unsigned long long) count);
			case PrimitiveMethod:
				return makeString("method%llu_t",
					(unsigned long long) count);
			case PrimitiveTemplatedMethod:
				return makeString("templatedmethod%llu_t",
					(unsigned long long) count);
			case PrimitiveInterfaceMethod:
				return makeString("interfacemethod%llu_t",
					(unsigned long long) count);
			case PrimitiveStaticInterfaceMethod:
				return makeString("staticinterfacemethod%llu_t",
					(unsigned long long) count);
			default:
				return toString();
		}
	}
	
	const char* PrimitiveID::toCString() const {
		switch (value_) {
			case PrimitiveVoid:
				return "void_t";
			case PrimitiveNull:
				return "null_t";
			case PrimitiveBool:
				return "bool";
			case PrimitiveCompareResult:
				return "compare_result_t";
			case PrimitiveFunctionPtr:
				return "function_ptr_t";
			case PrimitiveMethodFunctionPtr:
				return "methodfunction_ptr_t";
			case PrimitiveTemplatedFunctionPtr:
				return "templatedfunction_ptr_t";
			case PrimitiveTemplatedMethodFunctionPtr:
				return "templatedmethodfunction_ptr_t";
			case PrimitiveVarArgFunctionPtr:
				return "varargfunction_ptr_t";
			case PrimitiveMethod:
				return "method_t";
			case PrimitiveTemplatedMethod:
				return "templatedmethod_t";
			case PrimitiveInterfaceMethod:
				return "interfacemethod_t";
			case PrimitiveStaticInterfaceMethod:
				return "staticinterfacemethod_t";
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
				return "__ref";
			case PrimitivePtr:
				return "__ptr";
			case PrimitivePtrLval:
				return "ptr_lval";
			case PrimitiveValueLval:
				return "value_lval";
			case PrimitiveFinalLval:
				return "final_lval";
			case PrimitiveTypename:
				return "typename_t";
			case PrimitiveCount:
				return "count";
			case PrimitiveCountIncl:
				return "count_incl";
			case PrimitiveRange:
				return "range";
			case PrimitiveRangeIncl:
				return "range_incl";
			case PrimitiveReverseRange:
				return "reverse_range";
			case PrimitiveReverseRangeIncl:
				return "reverse_range_incl";
			case PrimitiveStaticArray:
				return "static_array_t";
		}
		
		throw std::logic_error("Unknown primitive ID.");
	}
	
	std::string PrimitiveID::toString() const {
		return toCString();
	}
	
}
