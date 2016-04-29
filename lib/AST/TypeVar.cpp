#include <string>

#include <locic/Support/String.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/TypeVar.hpp>
#include <locic/AST/Value.hpp>

namespace locic {

	namespace AST {
	
		typedef std::vector<Node<TypeVar>> TypeVarList;
		
		TypeVar* TypeVar::NamedVar(Node<TypeDecl> type, const String name) {
			TypeVar* typeVar = new TypeVar(NAMEDVAR);
			typeVar->namedVar_.isFinal = false;
			typeVar->namedVar_.isOverrideConst = false;
			typeVar->namedVar_.isUnused = false;
			typeVar->namedVar_.type = std::move(type);
			typeVar->namedVar_.name = name;
			return typeVar;
		}
			
		TypeVar* TypeVar::PatternVar(Node<TypeDecl> type, Node<TypeVarList> typeVarList) {
			TypeVar* typeVar = new TypeVar(PATTERNVAR);
			typeVar->patternVar_.type = std::move(type);
			typeVar->patternVar_.typeVarList = std::move(typeVarList);
			return typeVar;
		}
			
		TypeVar* TypeVar::Any() {
			return new TypeVar(ANYVAR);
		}
		
		TypeVar::TypeVar(const Kind pKind)
		: kind_(pKind) { }
		
		TypeVar::Kind TypeVar::kind() const {
			return kind_;
		}
		
		bool TypeVar::isNamed() const {
			return kind() == NAMEDVAR;
		}
		
		const Node<TypeDecl>& TypeVar::namedType() const {
			assert(isNamed());
			return namedVar_.type;
		}
		
		const String& TypeVar::name() const {
			assert(isNamed());
			return namedVar_.name;
		}
		
		bool TypeVar::isFinal() const {
			assert(isNamed());
			return namedVar_.isFinal;
		}
		
		void TypeVar::setFinal() {
			assert(isNamed());
			namedVar_.isFinal = true;
		}
		
		bool TypeVar::isOverrideConst() const {
			assert(isNamed());
			return namedVar_.isOverrideConst;
		}
		
		void TypeVar::setOverrideConst() {
			assert(isNamed());
			namedVar_.isOverrideConst = true;
		}
		
		bool TypeVar::isUnused() const {
			assert(isNamed());
			return namedVar_.isUnused;
		}
		
		void TypeVar::setUnused() {
			assert(isNamed());
			namedVar_.isUnused = true;
		}
		
		bool TypeVar::isPattern() const {
			return kind() == PATTERNVAR;
		}
		
		const Node<TypeDecl>& TypeVar::patternType() const {
			assert(isPattern());
			return patternVar_.type;
		}
		
		const Node<TypeVarList>& TypeVar::typeVarList() const {
			assert(isPattern());
			return patternVar_.typeVarList;
		}
		
		bool TypeVar::isAny() const {
			return kind() == ANYVAR;
		}
			
		std::string TypeVar::toString() const {
			switch (kind()) {
				case NAMEDVAR:
					return makeString("TypeVar[NAMED](type = %s, name = %s)",
						namedType().toString().c_str(), name().c_str());
				case PATTERNVAR:
					return makeString("TypeVar[PATTERN](type = %s, typeVarList = %s)",
						patternType().toString().c_str(),
						makeArrayString(*(typeVarList())).c_str());
				case ANYVAR:
					return "TypeVar[ANY]()";
				default:
					return "TypeVar[UNKNOWN]()";
			}
		}
		
	}
	
}

