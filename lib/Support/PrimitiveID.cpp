#include <cassert>
#include <string>

#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
	PrimitiveID PrimitiveID::Callable(const PrimitiveIDEnum baseValue,
	                                  const size_t argumentCount) {
		assert(argumentCount <= 8);
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
	
	const char* PrimitiveID::toCString() const {
		PRIMITIVE_STRING_CASE(value_, /*prefix=*/"");
	}
	
	std::string PrimitiveID::toString() const {
		return toCString();
	}
	
}
