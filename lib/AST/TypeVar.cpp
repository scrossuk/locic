#include <string>

#include <locic/String.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeVar.hpp>

namespace locic {

	namespace AST {
	
		struct TypeVar;
		
		typedef std::vector<Node<TypeVar>> TypeVarList;
		
		TypeVar::TypeVar(Kind pKind)
			: kind(pKind) { }
		
		TypeVar* TypeVar::NamedVar(const Node<Type>& type, const std::string& name) {
			TypeVar* typeVar = new TypeVar(NAMEDVAR);
			typeVar->namedVar.isFinal = false;
			typeVar->namedVar.type = type;
			typeVar->namedVar.name = name;
			return typeVar;
		}
		
		TypeVar* TypeVar::FinalNamedVar(const Node<Type>& type, const std::string& name) {
			TypeVar* typeVar = new TypeVar(NAMEDVAR);
			typeVar->namedVar.isFinal = true;
			typeVar->namedVar.type = type;
			typeVar->namedVar.name = name;
			return typeVar;
		}
			
		TypeVar* TypeVar::PatternVar(const Node<Type>& type, const Node<TypeVarList>& typeVarList) {
			TypeVar* typeVar = new TypeVar(PATTERNVAR);
			typeVar->patternVar.type = type;
			typeVar->patternVar.typeVarList = typeVarList;
			return typeVar;
		}
			
		TypeVar* TypeVar::Any() {
			return new TypeVar(ANYVAR);
		}
			
		std::string TypeVar::toString() const {
			switch(kind) {
				case NAMEDVAR:
					return makeString("TypeVar[NAMED](type = %s, name = %s)",
						namedVar.type.toString().c_str(), namedVar.name.c_str());
											 
				case PATTERNVAR:
					return makeString("TypeVar[PATTERN](type = %s, typeVarList = %s)",
						patternVar.type->toString().c_str(),
						makeArrayString(*(patternVar.typeVarList)).c_str());
											 
				case ANYVAR:
					return "TypeVar[ANY]()";
					
				default:
					return "TypeVar[UNKNOWN]()";
			}
		}
		
	}
	
}

