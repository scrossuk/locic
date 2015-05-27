#include <string>

#include <locic/Support/MakeString.hpp>
#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
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
		}
		
		throw std::logic_error("Unknown primitive ID.");
	}
	
	std::string PrimitiveID::toString() const {
		return toCString();
	}
	
}
