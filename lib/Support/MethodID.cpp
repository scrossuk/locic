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
			case METHOD_ASCIIVALUE:
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
	
	const char* MethodID::toCString() const {
		switch (value_) {
			case METHOD_CREATE:
				return "create";
			case METHOD_DEAD:
				return "__dead";
			case METHOD_NULL:
				return "null";
			case METHOD_ZERO:
				return "zero";
			case METHOD_UNIT:
				return "unit";
			case METHOD_LEADINGONES:
				return "leadingones";
			case METHOD_TRAILINGONES:
				return "trailingones";
			case METHOD_LEADINGZEROES:
				return "leadingzeroes";
			case METHOD_TRAILINGZEROES:
				return "trailingzeroes";
			case METHOD_IMPLICITCASTFROM:
				PRIMITIVE_STRING_CASE(primitiveID(), /*prefix=*/"implicitcast");
			case METHOD_CASTFROM:
				PRIMITIVE_STRING_CASE(primitiveID(), /*prefix=*/"cast");
			case METHOD_ALIGNMASK:
				return "__alignmask";
			case METHOD_SIZEOF:
				return "__sizeof";
			case METHOD_UNINITIALIZED:
				return "uninitialized";
			
			case METHOD_DESTROY:
				return "__destroy";
			case METHOD_IMPLICITCAST:
				return "implicitcast";
			case METHOD_CAST:
				return "cast";
			case METHOD_IMPLICITCOPY:
				return "implicitcopy";
			case METHOD_COPY:
				return "copy";
			case METHOD_PLUS:
				return "plus";
			case METHOD_MINUS:
				return "minus";
			case METHOD_NOT:
				return "not";
			case METHOD_FRONT:
				return "front";
			case METHOD_SKIPFRONT:
				return "skipfront";
			case METHOD_BACK:
				return "back";
			case METHOD_SKIPBACK:
				return "skipback";
			case METHOD_EMPTY:
				return "empty";
			case METHOD_ISZERO:
				return "iszero";
			case METHOD_ISPOSITIVE:
				return "ispositive";
			case METHOD_ISNEGATIVE:
				return "isnegative";
			case METHOD_ABS:
				return "abs";
			case METHOD_ADDRESS:
				return "address";
			case METHOD_DEREF:
				return "deref";
			case METHOD_DISSOLVE:
				return "dissolve";
			case METHOD_MOVE:
				return "move";
			case METHOD_SIGNEDVALUE:
				return "signedvalue";
			case METHOD_UNSIGNEDVALUE:
				return "unsignedvalue";
			case METHOD_COUNTLEADINGZEROES:
				return "countleadingzeroes";
			case METHOD_COUNTLEADINGONES:
				return "countleadingones";
			case METHOD_COUNTTRAILINGZEROES:
				return "counttrailingzeroes";
			case METHOD_COUNTTRAILINGONES:
				return "counttrailingones";
			case METHOD_SQRT:
				return "sqrt";
			case METHOD_INCREMENT:
				return "increment";
			case METHOD_DECREMENT:
				return "decrement";
			case METHOD_SETDEAD:
				return "__setdead";
			case METHOD_ISLIVE:
				return "__islive";
			case METHOD_SETINVALID:
				return "__setinvalid";
			case METHOD_ISVALID:
				return "__isvalid";
			case METHOD_ISEQUAL:
				return "isequal";
			case METHOD_ISNOTEQUAL:
				return "isnotequal";
			case METHOD_ISLESSTHAN:
				return "islessthan";
			case METHOD_ISLESSTHANOREQUAL:
				return "islessthanorequal";
			case METHOD_ISGREATERTHAN:
				return "isgreaterthan";
			case METHOD_ISGREATERTHANOREQUAL:
				return "isgreaterthanorequal";
			case METHOD_ASCIIVALUE:
				return "asciivalue";
			
			case METHOD_ADD:
				return "add";
			case METHOD_SUBTRACT:
				return "subtract";
			case METHOD_MULTIPLY:
				return "multiply";
			case METHOD_DIVIDE:
				return "divide";
			case METHOD_MODULO:
				return "modulo";
			case METHOD_COMPARE:
				return "compare";
			case METHOD_ASSIGN:
				return "assign";
			case METHOD_INDEX:
				return "index";
			case METHOD_EQUAL:
				return "equal";
			case METHOD_NOTEQUAL:
				return "notequal";
			case METHOD_LESSTHAN:
				return "lessthan";
			case METHOD_LESSTHANOREQUAL:
				return "lessthanorequal";
			case METHOD_GREATERTHAN:
				return "greaterthan";
			case METHOD_GREATERTHANOREQUAL:
				return "greaterthanorequal";
			case METHOD_BITWISEAND:
				return "bitwiseand";
			case METHOD_BITWISEOR:
				return "bitwiseor";
			case METHOD_LEFTSHIFT:
				return "leftshift";
			case METHOD_RIGHTSHIFT:
				return "rightshift";
			
			case METHOD_CALL:
				return "call";
			case METHOD_MOVETO:
				return "__moveto";
			case METHOD_INRANGE:
				return "inrange";
			case METHOD_SETVALUE:
				return "setvalue";
			case METHOD_EXTRACTVALUE:
				return "extractvalue";
			case METHOD_DESTROYVALUE:
				return "destroyvalue";
			
			case METHOD_MIN:
				return "min";
			case METHOD_MAX:
				return "max";
			case METHOD_RANGE:
				return "range";
			case METHOD_RANGE_INCL:
				return "rangeincl";
			case METHOD_REVERSE_RANGE:
				return "reverserange";
			case METHOD_REVERSE_RANGE_INCL:
				return "reverserangeincl";
		}
		
		locic_unreachable("Unknown Method ID.");
	}
	
	std::string MethodID::toString() const {
		return toCString();
	}
	
}
