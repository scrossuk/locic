#include <string>
#include <vector>
#include <locic/AST/Node.hpp>
#include <locic/AST/Symbol.hpp>
#include <locic/AST/Type.hpp>

namespace locic {

	namespace AST {
	
		Type::~Type() { }
		
		std::string Type::toString() const {
			switch(typeEnum) {
				case AUTO:
					return "auto";
					
				case CONST:
					return std::string("const ") + getConstTarget()->toString();
					
				case CONSTPREDICATE:
					return std::string("const<[TODO]> ") + getConstPredicateTarget()->toString();
					
				case NOTAG:
					return std::string("notag(") + getNoTagTarget()->toString() + ")";
					
				case LVAL:
					return std::string("lval <") + getLvalTarget()->toString() + "> " + getLvalType()->toString();
					
				case REF:
					return std::string("ref <") + getRefTarget()->toString() + "> " + getRefType()->toString();
					
				case STATICREF:
					return std::string("staticref <") + getStaticRefTarget()->toString() + "> " + getStaticRefType()->toString();
					
				case VOID:
					return "void";
					
				case BOOL:
					return "bool";
				
				case INTEGER: {
					const auto signedString = (integerSignedModifier() == SIGNED ?
						"signed" :
							(integerSignedModifier() == UNSIGNED ?
								"unsigned" :
								""
							)
						);
					return std::string("[integer type: ") + signedString + " " + integerName().asStdString() + std::string("]");	
				}
				
				case FLOAT: {
					return std::string("[float type: ") + floatName().asStdString() + std::string("]");	
				}
				
				case PRIMITIVE: {
					return std::string("[primitive type: ") + primitiveID().toString() + std::string("]");
				}
				
				case OBJECT:
					return std::string("[object type: ") + objectType.symbol->toString() + std::string("]");
					
				case REFERENCE:
					return getReferenceTarget()->toString() + "&";
					
				case POINTER:
					return getPointerTarget()->toString() + "*";
					
				case STATICARRAY:
					return getStaticArrayTarget()->toString() + "[" + getArraySize()->toString() + "]";
					
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
			}
			
			std::terminate();
		}
		
	}
	
}

