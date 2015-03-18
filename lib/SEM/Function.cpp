#include <locic/Support/String.hpp>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Function::Function(Name pName, ModuleScope pModuleScope)
			: isPrimitive_(false),
			  isMethod_(false),
			  isStaticMethod_(false),
			  type_(nullptr),
			  name_(std::move(pName)),
			  constPredicate_(Predicate::False()),
			  requiresPredicate_(Predicate::True()),
			  moduleScope_(std::move(pModuleScope)) { }
		
		const Name& Function::name() const {
			return name_;
		}
		
		void Function::setType(const Type* pType) {
			assert(type_ == nullptr);
			type_ = pType;
		}
		
		const Type* Function::type() const {
			return type_;
		}
		
		const ModuleScope& Function::moduleScope() const {
			return moduleScope_;
		}
		
		bool Function::isDeclaration() const {
			return !isDefinition();
		}
		
		bool Function::isDefinition() const {
			return scope_.get() != nullptr;
		}
		
		void Function::setPrimitive(bool pIsPrimitive) {
			isPrimitive_ = pIsPrimitive;
		}
		
		bool Function::isPrimitive() const {
			return isPrimitive_;
		}
		
		void Function::setMethod(bool pIsMethod) {
			isMethod_ = pIsMethod;
		}
		
		bool Function::isMethod() const {
			return isMethod_;
		}
		
		void Function::setStaticMethod(bool pIsStaticMethod) {
			// isStaticMethod() implies isMethod().
			assert(!pIsStaticMethod || isMethod());
			isStaticMethod_ = pIsStaticMethod;
		}
		
		bool Function::isStaticMethod() const {
			return isStaticMethod_;
		}
		
		TemplateVarArray& Function::templateVariables() {
			return templateVariables_;
		}
		
		const TemplateVarArray& Function::templateVariables() const {
			return templateVariables_;
		}
		
		FastMap<String, TemplateVar*>& Function::namedTemplateVariables() {
			return namedTemplateVariables_;
		}
		
		const FastMap<String, TemplateVar*>& Function::namedTemplateVariables() const {
			return namedTemplateVariables_;
		}
		
		const Predicate& Function::constPredicate() const {
			return constPredicate_;
		}
		
		void Function::setConstPredicate(Predicate predicate) {
			constPredicate_ = std::move(predicate);
		}
		
		const Predicate& Function::requiresPredicate() const {
			return requiresPredicate_;
		}
		
		void Function::setRequiresPredicate(Predicate predicate) {
			requiresPredicate_ = std::move(predicate);
		}
		
		void Function::setParameters(std::vector<Var*> pParameters) {
			parameters_ = std::move(pParameters);
		}
		
		const std::vector<Var*>& Function::parameters() const {
			return parameters_;
		}
		
		FastMap<String, Var*>& Function::namedVariables() {
			return namedVariables_;
		}
		
		const FastMap<String, Var*>& Function::namedVariables() const {
			return namedVariables_;
		}
		
		const Scope& Function::scope() const {
			assert(isDefinition());
			return *scope_;
		}
		
		void Function::setScope(std::unique_ptr<Scope> newScope) {
			assert(scope_.get() == nullptr);
			assert(newScope.get() != nullptr);
			scope_ = std::move(newScope);
		}
		
		void Function::setDebugInfo(const Debug::FunctionInfo newDebugInfo) {
			debugInfo_ = make_optional(newDebugInfo);
		}
		
		Optional<Debug::FunctionInfo> Function::debugInfo() const {
			return debugInfo_;
		}
		
		std::string Function::toString() const {
			if (isDeclaration()) {
				return makeString("FunctionDeclaration(name: %s, isMethod: %s, isStatic: %s, constSpecifier: %s, type: %s)",
								  name().toString().c_str(),
								  isMethod() ? "Yes" : "No",
								  isStaticMethod() ? "Yes" : "No",
								  constPredicate().toString().c_str(),
								  type()->toString().c_str());
			} else {
				return makeString("FunctionDefinition(name: %s, isMethod: %s, isStatic: %s, constSpecifier: %s, type: %s, scope: %s)",
								  name().toString().c_str(),
								  isMethod() ? "Yes" : "No",
								  isStaticMethod() ? "Yes" : "No",
								  constPredicate().toString().c_str(),
								  type()->toString().c_str(),
								  scope().toString().c_str());
			}
		}
		
	}
	
}

