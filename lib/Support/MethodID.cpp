#include <string>

#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/MethodID.hpp>

namespace locic {
	
	MethodID::MethodKind MethodID::kind() const {
		switch (value_) {
			case METHOD_CREATE:
			case METHOD_DEAD:
			case METHOD_NULL:
			case METHOD_ZERO:
			case METHOD_UNIT:
			case METHOD_LEADINGONES:
			case METHOD_TRAILINGONES:
			case METHOD_LEADINGZEROES:
			case METHOD_TRAILINGZEROES:
			case METHOD_IMPLICITCASTFROM:
			case METHOD_CASTFROM:
			case METHOD_ALIGNMASK:
			case METHOD_SIZEOF:
			case METHOD_UNINITIALIZED:
				return CONSTRUCTOR;
			
			case METHOD_DESTROY:
			case METHOD_IMPLICITCAST:
			case METHOD_CAST:
			case METHOD_IMPLICITCOPY:
			case METHOD_COPY:
			case METHOD_PLUS:
			case METHOD_MINUS:
			case METHOD_NOT:
			case METHOD_FRONT:
			case METHOD_SKIPFRONT:
			case METHOD_BACK:
			case METHOD_SKIPBACK:
			case METHOD_EMPTY:
			case METHOD_ISZERO:
			case METHOD_ISPOSITIVE:
			case METHOD_ISNEGATIVE:
			case METHOD_ABS:
			case METHOD_ADDRESS:
			case METHOD_DEREF:
			case METHOD_DISSOLVE:
			case METHOD_MOVE:
			case METHOD_SIGNEDVALUE:
			case METHOD_UNSIGNEDVALUE:
			case METHOD_COUNTLEADINGZEROES:
			case METHOD_COUNTLEADINGONES:
			case METHOD_COUNTTRAILINGZEROES:
			case METHOD_COUNTTRAILINGONES:
			case METHOD_SQRT:
			case METHOD_INCREMENT:
			case METHOD_DECREMENT:
			case METHOD_SETDEAD:
			case METHOD_ISLIVE:
			case METHOD_SETINVALID:
			case METHOD_ISVALID:
			case METHOD_ISEQUAL:
			case METHOD_ISNOTEQUAL:
			case METHOD_ISLESSTHAN:
			case METHOD_ISLESSTHANOREQUAL:
			case METHOD_ISGREATERTHAN:
			case METHOD_ISGREATERTHANOREQUAL:
				return UNARY;
			
			case METHOD_ADD:
			case METHOD_SUBTRACT:
			case METHOD_MULTIPLY:
			case METHOD_DIVIDE:
			case METHOD_MODULO:
			case METHOD_COMPARE:
			case METHOD_ASSIGN:
			case METHOD_INDEX:
			case METHOD_EQUAL:
			case METHOD_NOTEQUAL:
			case METHOD_LESSTHAN:
			case METHOD_LESSTHANOREQUAL:
			case METHOD_GREATERTHAN:
			case METHOD_GREATERTHANOREQUAL:
			case METHOD_BITWISEAND:
			case METHOD_BITWISEOR:
			case METHOD_LEFTSHIFT:
			case METHOD_RIGHTSHIFT:
				return BINARY;
			
			case METHOD_CALL:
			case METHOD_MOVETO:
			case METHOD_INRANGE:
			case METHOD_SETVALUE:
			case METHOD_EXTRACTVALUE:
			case METHOD_DESTROYVALUE:
				return UTIL;
			
			case METHOD_MIN:
			case METHOD_MAX:
			case METHOD_RANGE:
			case METHOD_RANGE_INCL:
			case METHOD_REVERSE_RANGE:
			case METHOD_REVERSE_RANGE_INCL:
				return FUNCTION;
		}
		
		locic_unreachable("Unknown Method ID.");
	}
	
	bool MethodID::isConstructor() const {
		return kind() == CONSTRUCTOR;
	}
	
	bool MethodID::isUnary() const {
		return kind() == UNARY;
	}
	
	bool MethodID::isBinary() const {
		return kind() == BINARY;
	}
	
	bool MethodID::isStandaloneFunction() const {
		return kind() == FUNCTION;
	}
	
	PrimitiveID MethodID::primitiveID() const {
		return primitiveID_;
	}
	
	std::string MethodID::toString() const {
		return toCString();
	}
	
}
