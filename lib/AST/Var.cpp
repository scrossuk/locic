#include <string>

#include <locic/Support/String.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>

namespace locic {

	namespace AST {
	
		typedef std::vector<Node<Var>> VarList;
		
		Var* Var::Any(Node<TypeDecl> type) {
			return new Var(ANYVAR, std::move(type));
		}
		
		Var* Var::NamedVar(Node<TypeDecl> type, const String name) {
			Var* typeVar = new Var(NAMEDVAR, std::move(type));
			typeVar->namedVar_.name = name;
			return typeVar;
		}
		
		Var* Var::PatternVar(Node<TypeDecl> type, Node<VarList> varList) {
			Var* typeVar = new Var(PATTERNVAR, std::move(type));
			typeVar->patternVar_.varList = std::move(varList);
			return typeVar;
		}
		
		Var::Var(const Kind pKind, Node<TypeDecl> argType)
		: kind_(pKind), isFinal_(false), isOverrideConst_(false),
		isMarkedUnused_(false), isUsed_(false), index_(-1),
		type_(std::move(argType)), constructType_(nullptr),
		lvalType_(nullptr) { }
		
		Var::~Var() { }
		
		Var::Kind Var::kind() const {
			return kind_;
		}
		
		Node<TypeDecl>& Var::declType() {
			return type_;
		}
		
		const Node<TypeDecl>& Var::declType() const {
			return type_;
		}
		
		bool Var::isAny() const {
			return kind() == ANYVAR;
		}
		
		bool Var::isNamed() const {
			return kind() == NAMEDVAR;
		}
		
		const String& Var::name() const {
			assert(isNamed());
			return namedVar_.name;
		}
		
		bool Var::isPattern() const {
			return kind() == PATTERNVAR;
		}
		
		Node<VarList>& Var::varList() {
			assert(isPattern());
			return patternVar_.varList;
		}
		
		const Node<VarList>& Var::varList() const {
			assert(isPattern());
			return patternVar_.varList;
		}
		
		bool Var::isFinal() const {
			assert(isNamed());
			return isFinal_;
		}
		
		void Var::setFinal() {
			assert(isNamed());
			isFinal_ = true;
		}
		
		bool Var::isOverrideConst() const {
			assert(isNamed());
			return isOverrideConst_;
		}
		
		void Var::setOverrideConst() {
			assert(isNamed());
			isOverrideConst_ = true;
		}
		
		bool Var::isMarkedUnused() const {
			assert(isNamed());
			return isMarkedUnused_;
		}
		
		void Var::setMarkedUnused() {
			assert(isNamed());
			isMarkedUnused_ = true;
		}
		
		bool Var::isUsed() const {
			assert(isNamed());
			return isUsed_;
		}
		
		void Var::setUsed() {
			assert(isNamed());
			isUsed_ = true;
		}
		
		const AST::Type* Var::constructType() const {
			assert(constructType_ != nullptr);
			return constructType_;
		}
		
		void Var::setConstructType(const AST::Type* type) {
			assert(constructType_ == nullptr && type != nullptr);
			constructType_ = type;
		}
		
		const AST::Type* Var::lvalType() const {
			assert(lvalType_ != nullptr);
			return lvalType_;
		}
		
		void Var::setLvalType(const AST::Type* type) {
			assert(lvalType_ == nullptr && type != nullptr);
			lvalType_ = type;
		}
		
		size_t Var::index() const {
			assert(index_ != (size_t) -1);
			return index_;
		}
		
		void Var::setIndex(const size_t argIndex) {
			assert(index_ == (size_t) -1);
			index_ = argIndex;
		}
		
		void Var::setDebugInfo(const Debug::VarInfo newDebugInfo) {
			debugInfo_ = make_optional(newDebugInfo);
		}
		
		Optional<Debug::VarInfo> Var::debugInfo() const {
			return debugInfo_;
		}
			
		std::string Var::toString() const {
			switch (kind()) {
				case NAMEDVAR:
					return makeString("Var[NAMED](type = %s, name = %s)",
						declType().toString().c_str(), name().c_str());
				case PATTERNVAR:
					return makeString("Var[PATTERN](type = %s, typeVarList = %s)",
						declType().toString().c_str(),
						makeArrayString(*(varList())).c_str());
				case ANYVAR:
					return "Var[ANY]()";
			}
			
			locic_unreachable("Unknown var kind.");
		}
		
	}
	
}

