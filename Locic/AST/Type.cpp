#include <string>
#include <vector>
#include <Locic/AST/Node.hpp>
#include <Locic/AST/Symbol.hpp>
#include <Locic/AST/Type.hpp>

namespace Locic {

	namespace AST {
	
		std::string Type::toString() const {
			switch(typeEnum) {
				case NONE:
					return "[NONE]";
					
				case UNDEFINED:
					return "auto";
					
				case BRACKET:
					return std::string("(") + getBracketTarget()->toString() + ")";
					
				case CONST:
					return std::string("const ") + getConstTarget()->toString();
					
				case VOID:
					return "void";
					
				case NULLT:
					return "null";
					
				case OBJECT:
					return std::string("[object type: ") + objectType.symbol->toString() + std::string("]");
					
				case REFERENCE:
					return getReferenceTarget()->toString() + "&";
					
				case FUNCTION: {
					std::string str;
					str += "(";
					str += functionType.returnType->toString();
					str += ")(";
					
					for(size_t i = 0; i < functionType.parameterTypes->size(); i++) {
						if(i != 0) {
							str += ", ";
						}
						
						str += functionType.parameterTypes->at(i)->toString();
					}
					
					str += ")";
					return str;
				}
				
				default:
					return "[UNKNOWN]";
			}
		}
		
	}
	
}

