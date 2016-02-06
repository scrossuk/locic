#include <string>

#include <locic/AST/Value.hpp>
#include <locic/Support/MakeString.hpp>

namespace locic {
	
	namespace AST {
		
		Value::~Value() { }
		
		Value::TypeEnum Value::kind() const {
			return typeEnum;
		}
		
		std::string Value::toString() const {
			switch (kind()) {
				case SELF:
					return "self";
				case THIS:
					return "this";
				case BRACKET:
					return "bracket";
				case LITERAL:
					return "literal";
				case SYMBOLREF:
					return "symbolref";
				case TYPEREF:
					return "typeref";
				case MEMBERREF:
					return "memberref";
				case ALIGNOF:
					return "alignof()";
				case SIZEOF:
					return "sizeof()";
				case UNARYOP:
					return "unary";
				case BINARYOP:
					return "binary";
				case TERNARY:
					return "ternary";
				case CAST:
					return "cast";
				case LVAL:
					return "lval";
				case NOLVAL:
					return "nolval";
				case REF:
					return "ref";
				case NOREF:
					return "noref";
				case INTERNALCONSTRUCT:
					return "@()";
				case MEMBERACCESS:
					return "memberaccess";
				case TEMPLATEDMEMBERACCESS:
					return "templatedmemberaccess";
				case FUNCTIONCALL:
					return "call";
				case CAPABILITYTEST:
					return "capabilitytest";
				case ARRAYLITERAL:
					return "arrayliteral";
				case MERGE:
					return makeString("Merge(%s, %s)",
					                  merge.first.toString().c_str(),
					                  merge.second.toString().c_str());
			}
		}
		
	}
	
}

