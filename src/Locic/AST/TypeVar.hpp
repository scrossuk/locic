#ifndef LOCIC_AST_TYPEVAR_HPP
#define LOCIC_AST_TYPEVAR_HPP

#include <string>
#include <Locic/Optional.hpp>
#include <Locic/String.hpp>
#include <Locic/AST/Node.hpp>
#include <Locic/AST/Type.hpp>

namespace AST {

	struct TypeVar;
	
	typedef std::vector<Node<TypeVar>> TypeVarList;
	
	struct TypeVar {
		enum Kind {
			NAMEDVAR,
			PATTERNVAR,
			ANYVAR
		} kind;
		
		struct {
			Node<Type> type;
			std::string name;
			bool usesCustomLval;
		} namedVar;
		
		struct {
			Node<Type> type;
			Node<TypeVarList> typeVarList;
		} patternVar;
		
		inline TypeVar(Kind pKind)
			: kind(pKind) { }
		
		inline static TypeVar* NamedVar(const Node<Type>& type, const std::string& name, bool usesCustomLval) {
			TypeVar* typeVar = new TypeVar(NAMEDVAR);
			typeVar->namedVar.type = type;
			typeVar->namedVar.name = name;
			typeVar->namedVar.usesCustomLval = usesCustomLval;
			return typeVar;
		}
		
		inline static TypeVar* PatternVar(const Node<Type>& type, const Node<TypeVarList>& typeVarList) {
			TypeVar* typeVar = new TypeVar(PATTERNVAR);
			typeVar->patternVar.type = type;
			typeVar->patternVar.typeVarList = typeVarList;
			return typeVar;
		}
		
		inline static TypeVar* Any() {
			return new TypeVar(ANYVAR);
		}
		
		inline std::string toString() const {
			switch (kind) {
				case NAMEDVAR:
					return Locic::makeString("TypeVar[NAMED](isLval = %s, type = %s, name = %s)",
						namedVar.usesCustomLval ? "YES" : "NO",
						namedVar.type.toString().c_str(), namedVar.name.c_str());
				case PATTERNVAR:
					return Locic::makeString("TypeVar[PATTERN](type = %s, typeVarList = %s)",
						patternVar.type->toString().c_str(),
						Locic::makeArrayString(*(patternVar.typeVarList)).c_str());
				case ANYVAR:
					return "TypeVar[ANY]()";
				default:
					return "TypeVar[UNKNOWN]()";
			}
		}
	};
	
}

#endif
