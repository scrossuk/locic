#include <locic/AST/Alias.hpp>
#include <locic/AST/Function.hpp>
#include <locic/AST/Namespace.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		ScopeElement ScopeElement::Namespace(AST::Namespace& nameSpace) {
			ScopeElement element(NAMESPACE);
			element.data_.nameSpace = &nameSpace;
			return element;
		}
		
		ScopeElement ScopeElement::Alias(AST::Alias& alias) {
			ScopeElement element(ALIAS);
			element.data_.alias = &alias;
			return element;
		}
		
		ScopeElement ScopeElement::TypeInstance(AST::TypeInstance& typeInstance) {
			ScopeElement element(TYPEINSTANCE);
			element.data_.typeInstance = &typeInstance;
			return element;
		}
		
		ScopeElement ScopeElement::Function(AST::Function& function) {
			ScopeElement element(FUNCTION);
			element.data_.function = &function;
			return element;
		}
		
		ScopeElement ScopeElement::Scope(AST::Scope& scope) {
			ScopeElement element(SCOPE);
			element.data_.scope = &scope;
			return element;
		}
		
		ScopeElement ScopeElement::SwitchCase(AST::SwitchCase& switchCase) {
			ScopeElement element(SWITCHCASE);
			element.data_.switchCase = &switchCase;
			return element;
		}
		
		ScopeElement ScopeElement::CatchClause(AST::CatchClause& catchClause) {
			ScopeElement element(CATCHCLAUSE);
			element.data_.catchClause = &catchClause;
			return element;
		}
		
		ScopeElement ScopeElement::Loop() {
			return ScopeElement(LOOP);
		}
		
		ScopeElement ScopeElement::ScopeAction(const String& state) {
			ScopeElement element(SCOPEACTION);
			element.data_.scopeActionState = state;
			return element;
		}
		
		ScopeElement ScopeElement::TryScope() {
			return ScopeElement(TRYSCOPE);
		}
		
		ScopeElement ScopeElement::AssertNoExcept() {
			return ScopeElement(ASSERTNOEXCEPT);
		}
		
		ScopeElement::Kind ScopeElement::kind() const {
			return kind_;
		}
		
		bool ScopeElement::isNamespace() const {
			return kind() == NAMESPACE;
		}
		
		bool ScopeElement::isAlias() const {
			return kind() == ALIAS;
		}
		
		bool ScopeElement::isTypeInstance() const {
			return kind() == TYPEINSTANCE;
		}
		
		bool ScopeElement::isFunction() const {
			return kind() == FUNCTION;
		}
		
		bool ScopeElement::isScope() const {
			return kind() == SCOPE;
		}
		
		bool ScopeElement::isSwitchCase() const {
			return kind() == SWITCHCASE;
		}
		
		bool ScopeElement::isCatchClause() const {
			return kind() == CATCHCLAUSE;
		}
		
		bool ScopeElement::isLoop() const {
			return kind() == LOOP;
		}
		
		bool ScopeElement::isScopeAction() const {
			return kind() == SCOPEACTION;
		}
		
		bool ScopeElement::isTryScope() const {
			return kind() == TRYSCOPE;
		}
		
		bool ScopeElement::isAssertNoExcept() const {
			return kind() == ASSERTNOEXCEPT;
		}
		
		AST::Namespace& ScopeElement::nameSpace() const {
			assert(isNamespace());
			return *(data_.nameSpace);
		}
		
		AST::Alias& ScopeElement::alias() const {
			assert(isAlias());
			return *(data_.alias);
		}
		
		AST::TypeInstance& ScopeElement::typeInstance() const {
			assert(isTypeInstance());
			return *(data_.typeInstance);
		}
		
		AST::Function& ScopeElement::function() const {
			assert(isFunction());
			return *(data_.function);
		}
		
		AST::Scope& ScopeElement::scope() const {
			assert(isScope());
			return *(data_.scope);
		}
		
		AST::SwitchCase& ScopeElement::switchCase() const {
			assert(isSwitchCase());
			return *(data_.switchCase);
		}
		
		AST::CatchClause& ScopeElement::catchClause() const {
			assert(isCatchClause());
			return *(data_.catchClause);
		}
		
		const String& ScopeElement::scopeActionState() const {
			assert(isScopeAction());
			return data_.scopeActionState;
		}
		
		bool ScopeElement::hasName() const {
			return (isNamespace() && !nameSpace().name().empty()) || isAlias() || isTypeInstance() || isFunction();
		}
		
		const String& ScopeElement::name() const {
			assert(hasName());
			if (isNamespace()) {
				return nameSpace().name().last();
			} else if (isAlias()) {
				return alias().fullName().last();
			} else if (isTypeInstance()) {
				return typeInstance().fullName().last();
			} else if (isFunction()) {
				return function().fullName().last();
			} else {
				locic_unreachable("Can't access name of unnamed scope element.");
			}
		}
		
		ScopeElement::ScopeElement(Kind pKind)
			: kind_(pKind) {
				data_.ptr = nullptr;
			}
		
	}
	
}

