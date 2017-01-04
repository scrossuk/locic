#ifndef LOCIC_SUPPORT_METHODID_HPP
#define LOCIC_SUPPORT_METHODID_HPP

#include <cassert>
#include <string>

#include <locic/Support/ErrorHandling.hpp>
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
		METHOD_MOVE,
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
		METHOD_LVALMOVE,
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
		METHOD_ASCIIVALUE,
		
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
		
		const char* toCString() const;
		
		std::string toString() const;
		
	private:
		MethodIDEnum value_;
		PrimitiveID primitiveID_;
		
	};
	
}

#endif
