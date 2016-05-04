#include <string>

#include <locic/Support/String.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/Value.hpp>
#include <locic/AST/Var.hpp>

namespace locic {

	namespace AST {
	
		typedef std::vector<Node<Var>> VarList;
		
		Var* Var::NamedVar(Node<TypeDecl> type, const String name) {
			Var* typeVar = new Var(NAMEDVAR);
			typeVar->namedVar_.isFinal = false;
			typeVar->namedVar_.isOverrideConst = false;
			typeVar->namedVar_.isUnused = false;
			typeVar->namedVar_.type = std::move(type);
			typeVar->namedVar_.name = name;
			return typeVar;
		}
			
		Var* Var::PatternVar(Node<TypeDecl> type, Node<VarList> varList) {
			Var* typeVar = new Var(PATTERNVAR);
			typeVar->patternVar_.type = std::move(type);
			typeVar->patternVar_.varList = std::move(varList);
			return typeVar;
		}
			
		Var* Var::Any() {
			return new Var(ANYVAR);
		}
		
		Var::Var(const Kind pKind)
		: kind_(pKind) { }
		
		Var::Kind Var::kind() const {
			return kind_;
		}
		
		bool Var::isNamed() const {
			return kind() == NAMEDVAR;
		}
		
		Node<TypeDecl>& Var::namedType() {
			assert(isNamed());
			return namedVar_.type;
		}
		
		const Node<TypeDecl>& Var::namedType() const {
			assert(isNamed());
			return namedVar_.type;
		}
		
		const String& Var::name() const {
			assert(isNamed());
			return namedVar_.name;
		}
		
		bool Var::isFinal() const {
			assert(isNamed());
			return namedVar_.isFinal;
		}
		
		void Var::setFinal() {
			assert(isNamed());
			namedVar_.isFinal = true;
		}
		
		bool Var::isOverrideConst() const {
			assert(isNamed());
			return namedVar_.isOverrideConst;
		}
		
		void Var::setOverrideConst() {
			assert(isNamed());
			namedVar_.isOverrideConst = true;
		}
		
		bool Var::isUnused() const {
			assert(isNamed());
			return namedVar_.isUnused;
		}
		
		void Var::setUnused() {
			assert(isNamed());
			namedVar_.isUnused = true;
		}
		
		bool Var::isPattern() const {
			return kind() == PATTERNVAR;
		}
		
		Node<TypeDecl>& Var::patternType() {
			assert(isPattern());
			return patternVar_.type;
		}
		
		const Node<TypeDecl>& Var::patternType() const {
			assert(isPattern());
			return patternVar_.type;
		}
		
		Node<VarList>& Var::varList() {
			assert(isPattern());
			return patternVar_.varList;
		}
		
		const Node<VarList>& Var::varList() const {
			assert(isPattern());
			return patternVar_.varList;
		}
		
		bool Var::isAny() const {
			return kind() == ANYVAR;
		}
			
		std::string Var::toString() const {
			switch (kind()) {
				case NAMEDVAR:
					return makeString("Var[NAMED](type = %s, name = %s)",
						namedType().toString().c_str(), name().c_str());
				case PATTERNVAR:
					return makeString("Var[PATTERN](type = %s, typeVarList = %s)",
						patternType().toString().c_str(),
						makeArrayString(*(varList())).c_str());
				case ANYVAR:
					return "Var[ANY]()";
				default:
					return "Var[UNKNOWN]()";
			}
		}
		
	}
	
}

