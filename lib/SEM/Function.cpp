#include <locic/Support/String.hpp>

#include <locic/AST/Function.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
		
		namespace {
			
			AST::Node<AST::Function> createNamedASTFunction(Name name) {
				AST::Node<Name> nameNode(Debug::SourceLocation::Null(),
				                         new Name(std::move(name)));
				AST::Node<AST::RequireSpecifier> requireSpecifierNode(Debug::SourceLocation::Null(),
				                                                      AST::RequireSpecifier::None());
				const auto function = AST::Function::DefaultMethodDef(std::move(nameNode),
				                                                      std::move(requireSpecifierNode));
				return AST::Node<AST::Function>(Debug::SourceLocation::Null(),
				                                function);
			}
			
		}
		
		Function::Function(GlobalStructure pParent,
		                   Name pName,
		                   ModuleScope pModuleScope)
			: parent_(std::move(pParent)),
			  function_(createNamedASTFunction(std::move(pName))),
			  isDefault_(false),
			  isPrimitive_(false),
			  isMethod_(false),
			  isStaticMethod_(false),
			  constPredicate_(Predicate::False()),
			  requiresPredicate_(Predicate::True()),
			  moduleScope_(std::move(pModuleScope)) { }
		
		Function::Function(GlobalStructure pParent,
		                   AST::Node<AST::Function> argFunction,
		                   ModuleScope pModuleScope)
			: parent_(std::move(pParent)),
			  function_(std::move(argFunction)),
			  isDefault_(false),
			  isPrimitive_(false),
			  isMethod_(false),
			  isStaticMethod_(false),
			  constPredicate_(Predicate::False()),
			  requiresPredicate_(Predicate::True()),
			  moduleScope_(std::move(pModuleScope)) { }
		
		GlobalStructure& Function::parent() {
			return parent_;
		}
		
		const GlobalStructure& Function::parent() const {
			return parent_;
		}
		
		Namespace& Function::nameSpace() {
			return parent().nextNamespace();
		}
		
		const Namespace& Function::nameSpace() const {
			return parent().nextNamespace();
		}
		
		const AST::Node<AST::Function>& Function::function() const {
			return function_;
		}
		
		const Name& Function::name() const {
			return *(function()->name());
		}
		
		void Function::setType(const FunctionType pType) {
			type_ = pType;
		}
		
		const FunctionType& Function::type() const {
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
		
		void Function::setDefault(const bool pIsDefault) {
			assert(!(pIsDefault && isDefinition()) &&
			       "Functions can't be marked default and have definitions.");
			isDefault_ = pIsDefault;
		}
		
		bool Function::isDefault() const {
			return isDefault_;
		}
		
		void Function::setPrimitive(const bool pIsPrimitive) {
			isPrimitive_ = pIsPrimitive;
		}
		
		bool Function::isPrimitive() const {
			return isPrimitive_;
		}
		
		void Function::setMethod(const bool pIsMethod) {
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
		
		const Predicate& Function::noexceptPredicate() const {
			return type().attributes().noExceptPredicate();
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
			assert(!isDefault() &&
			       "Functions can't be marked default and have definitions.");
			scope_ = std::move(newScope);
		}
		
		void Function::setDebugInfo(Debug::FunctionInfo newDebugInfo) {
			debugInfo_ = make_optional(std::move(newDebugInfo));
		}
		
		const Optional<Debug::FunctionInfo>& Function::debugInfo() const {
			return debugInfo_;
		}
		
		std::string Function::toString() const {
			if (isDeclaration()) {
				return makeString("FunctionDeclaration(name: %s, isMethod: %s, isStatic: %s, constSpecifier: %s, requiresPredicate: %s, type: %s)",
								  name().toString().c_str(),
								  isMethod() ? "Yes" : "No",
								  isStaticMethod() ? "Yes" : "No",
								  constPredicate().toString().c_str(),
								  requiresPredicate().toString().c_str(),
								  type().toString().c_str());
			} else {
				return makeString("FunctionDefinition(name: %s, isMethod: %s, isStatic: %s, constSpecifier: %s, requiresPredicate: %s, type: %s, scope: %s)",
								  name().toString().c_str(),
								  isMethod() ? "Yes" : "No",
								  isStaticMethod() ? "Yes" : "No",
								  constPredicate().toString().c_str(),
								  requiresPredicate().toString().c_str(),
								  type().toString().c_str(),
								  scope().toString().c_str());
			}
		}
		
	}
	
}

