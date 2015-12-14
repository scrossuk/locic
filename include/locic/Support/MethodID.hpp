#ifndef LOCIC_SUPPORT_METHODID_HPP
#define LOCIC_SUPPORT_METHODID_HPP

#include <cassert>
#include <string>

#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
	enum MethodIDEnum {
		// Constructor methods.
		METHOD_CREATE,
		METHOD_DEAD,
		METHOD_NULL,
		METHOD_ZERO,
		METHOD_UNIT,
		METHOD_LEADINGONES,
		METHOD_TRAILINGONES,
		METHOD_LEADINGZEROES,
		METHOD_TRAILINGZEROES,
		METHOD_IMPLICITCASTFROM,
		METHOD_CASTFROM,
		METHOD_ALIGNMASK,
		METHOD_SIZEOF,
		METHOD_UNINITIALIZED,
		
		// Unary methods.
		METHOD_DESTROY,
		METHOD_IMPLICITCAST,
		METHOD_CAST,
		METHOD_IMPLICITCOPY,
		METHOD_COPY,
		METHOD_PLUS,
		METHOD_MINUS,
		METHOD_NOT,
		METHOD_FRONT,
		METHOD_SKIPFRONT,
		METHOD_BACK,
		METHOD_SKIPBACK,
		METHOD_EMPTY,
		METHOD_ISZERO,
		METHOD_ISPOSITIVE,
		METHOD_ISNEGATIVE,
		METHOD_ABS,
		METHOD_ADDRESS,
		METHOD_DEREF,
		METHOD_DISSOLVE,
		METHOD_MOVE,
		METHOD_SIGNEDVALUE,
		METHOD_UNSIGNEDVALUE,
		METHOD_COUNTLEADINGZEROES,
		METHOD_COUNTLEADINGONES,
		METHOD_COUNTTRAILINGZEROES,
		METHOD_COUNTTRAILINGONES,
		METHOD_SQRT,
		METHOD_INCREMENT,
		METHOD_DECREMENT,
		METHOD_SETDEAD,
		METHOD_ISLIVE,
		METHOD_SETINVALID,
		METHOD_ISVALID,
		METHOD_ISEQUAL,
		METHOD_ISNOTEQUAL,
		METHOD_ISLESSTHAN,
		METHOD_ISLESSTHANOREQUAL,
		METHOD_ISGREATERTHAN,
		METHOD_ISGREATERTHANOREQUAL,
		
		// Binary methods.
		METHOD_ADD,
		METHOD_SUBTRACT,
		METHOD_MULTIPLY,
		METHOD_DIVIDE,
		METHOD_MODULO,
		METHOD_COMPARE,
		METHOD_ASSIGN,
		METHOD_INDEX,
		METHOD_EQUAL,
		METHOD_NOTEQUAL,
		METHOD_LESSTHAN,
		METHOD_LESSTHANOREQUAL,
		METHOD_GREATERTHAN,
		METHOD_GREATERTHANOREQUAL,
		METHOD_BITWISEAND,
		METHOD_BITWISEOR,
		METHOD_LEFTSHIFT,
		METHOD_RIGHTSHIFT,
		
		// Util methods.
		METHOD_CALL,
		METHOD_MOVETO,
		METHOD_INRANGE,
		METHOD_SETVALUE,
		METHOD_EXTRACTVALUE,
		METHOD_DESTROYVALUE,
		
		// Functions.
		METHOD_MIN,
		METHOD_MAX,
		METHOD_RANGE,
		METHOD_RANGE_INCL,
		METHOD_REVERSE_RANGE,
		METHOD_REVERSE_RANGE_INCL
	};
	
	/**
	 * \brief MethodID
	 * 
	 * This class uses an enum value to efficiently refer to known methods
	 * (e.g. 'implicitcopy').
	 * 
	 * Note that the 'cast_*' and 'implicitcast_*' methods have an
	 * associated primitive ID (which they're casting from).
	 */
	class MethodID {
	public:
		MethodID(const MethodIDEnum value,
		         const PrimitiveID argPrimitiveID = PrimitiveVoid)
		: value_(value),
		  primitiveID_(argPrimitiveID) {
			// Only 'cast_*' and 'implicitcast_*' can have non-void
			// associated primitive ID.
			assert(value == METHOD_CASTFROM ||
			       value == METHOD_IMPLICITCASTFROM ||
			       argPrimitiveID == PrimitiveVoid);
		}
		
		bool operator==(const MethodIDEnum other) const {
			return value_ == other;
		}
		
		bool operator!=(const MethodIDEnum other) const {
			return value_ != other;
		}
		
		bool operator==(const MethodID& other) const {
			return value_ == other.value_;
		}
		
		bool operator!=(const MethodID& other) const {
			return value_ != other.value_;
		}
		
		operator MethodIDEnum() const {
			return value_;
		}
		
		enum MethodKind {
			CONSTRUCTOR,
			UNARY,
			BINARY,
			UTIL,
			FUNCTION
		};
		
		MethodKind kind() const;
		
		bool isConstructor() const;
		
		bool isUnary() const;
		
		bool isBinary() const;
		
		bool isStandaloneFunction() const;
		
		PrimitiveID primitiveID() const;
		
		inline const char* toCString() const {
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
					// TODO: use primitiveID() here.
					return "implicitcast_?";
				case METHOD_CASTFROM:
					// TODO: use primitiveID() here.
					return "cast_?";
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
			
			throw std::logic_error("Unknown Method ID.");
		}
		
		std::string toString() const;
		
	private:
		MethodIDEnum value_;
		PrimitiveID primitiveID_;
		
	};
	
}

#endif
