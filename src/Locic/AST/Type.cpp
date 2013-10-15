#include <string>
#include <vector>
#include <Locic/AST/Node.hpp>
#include <Locic/AST/Symbol.hpp>
#include <Locic/AST/Type.hpp>

namespace AST {

	void Type::applyTransitiveConst() {
		Type* t = this;
		
		while (true) {
			t->isMutable = false;
			
			if (t->typeEnum == REFERENCE) {
				t = t->getReferenceTarget().get();
			} else {
				break;
			}
		}
	}
	
	std::string Type::toString() const {
		std::string str;
		
		bool bracket = false;
		
		if (!isMutable) {
			str += "const ";
			bracket = true;
		}
		
		if (bracket) {
			str += "(";
		}
		
		switch (typeEnum) {
			case UNDEFINED: {
				str += "[undefined]";
				break;
			}
			
			case VOID: {
				str += "void";
				break;
			}
			
			case NULLT: {
				str += "null";
				break;
			}
			
			case OBJECT:
				str += std::string("[object type: ") + objectType.symbol->toString() + std::string("]");
				break;
				
			case REFERENCE:
				str += getReferenceTarget()->toString();
				str += "&";
				break;
				
			case FUNCTION: {
				str += "(";
				str += functionType.returnType->toString();
				str += ")(";
				
				for (std::size_t i = 0; i < functionType.parameterTypes->size(); i++) {
					if (i != 0) {
						str += ", ";
					}
					
					str += functionType.parameterTypes->at(i)->toString();
				}
				
				str += ")";
				break;
			}
			
			default:
				break;
		}
		
		if (bracket) {
			str += ")";
		}
		
		return str;
	}
	
}

