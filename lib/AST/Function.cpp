#include <string>
#include <vector>

#include <locic/Support/Name.hpp>

#include <locic/AST/Function.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/Scope.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/Var.hpp>

namespace locic {

	namespace AST {
		
		Function::Function() :
		isDefinition_(false), isDefaultDefinition_(false), isVarArg_(false),
		isStatic_(false), isImported_(false), isExported_(false),
		isPrimitive_(false), templateVariables_(makeDefaultNode<TemplateVarList>()),
		semFunction_(nullptr) { }
		
		bool Function::isDeclaration() const {
			return !isDefinition_;
		}
		
		bool Function::isDefinition() const {
			return isDefinition_;
		}
		
		void Function::setIsDefinition(const bool value) {
			isDefinition_ = value;
		}
		
		bool Function::isDefaultDefinition() const {
			return isDefaultDefinition_;
		}
		
		void Function::setIsDefaultDefinition(const bool value) {
			assert(isDefinition());
			isDefaultDefinition_ = value;
		}
		
		bool Function::isStatic() const {
			return isStatic_;
		}
		
		void Function::setIsStatic(const bool value) {
			isStatic_ = value;
		}
		
		bool Function::isVarArg() const {
			return isVarArg_;
		}
		
		void Function::setIsVarArg(const bool value) {
			isVarArg_ = value;
		}
		
		bool Function::isImported() const {
			return isImported_;
		}
		
		void Function::setIsImported(const bool value) {
			isImported_ = value;
		}
		
		bool Function::isExported() const {
			return isExported_;
		}
		
		void Function::setIsExported(const bool value) {
			isExported_ = value;
		}
		
		bool Function::isPrimitive() const {
			return isPrimitive_;
		}
		
		void Function::setIsPrimitive(const bool value) {
			isPrimitive_ = value;
		}
		
		const Node<Name>& Function::name() const {
			return name_;
		}
		
		void Function::setName(Node<Name> pName) {
			name_ = std::move(pName);
		}
		
		Node<TypeDecl>& Function::returnType() {
			assert(!isDefaultDefinition());
			return returnType_;
		}
		
		const Node<TypeDecl>& Function::returnType() const {
			assert(!isDefaultDefinition());
			return returnType_;
		}
		
		void Function::setReturnType(Node<TypeDecl> pReturnType) {
			returnType_ = std::move(pReturnType);
		}
		
		const Node<VarList>& Function::parameters() const {
			assert(!isDefaultDefinition());
			return parameters_;
		}
		
		void Function::setParameters(Node<VarList> pParameters) {
			parameters_ = std::move(pParameters);
		}
		
		const Node<Scope>& Function::scope() const {
			assert(isDefinition() && !isDefaultDefinition());
			return scope_;
		}
		
		void Function::setScope(Node<Scope> pScope) {
			assert(isDefinition());
			scope_ = std::move(pScope);
		}
		
		const Node<ConstSpecifier>& Function::constSpecifier() const {
			return constSpecifier_;
		}
		
		void Function::setConstSpecifier(Node<ConstSpecifier> pConstSpecifier) {
			constSpecifier_ = std::move(pConstSpecifier);
		}
		
		const Node<RequireSpecifier>& Function::noexceptSpecifier() const {
			return noexceptSpecifier_;
		}
		
		void Function::setNoexceptSpecifier(Node<RequireSpecifier> pNoexceptSpecifier) {
			noexceptSpecifier_ = std::move(pNoexceptSpecifier);
		}
		
		const Node<RequireSpecifier>& Function::requireSpecifier() const {
			return requireSpecifier_;
		}
		
		void Function::setRequireSpecifier(Node<RequireSpecifier> pRequireSpecifier) {
			requireSpecifier_ = std::move(pRequireSpecifier);
		}
		
		const Node<TemplateVarList>& Function::templateVariables() const {
			return templateVariables_;
		}
		
		void Function::setTemplateVariables(Node<TemplateVarList> pTemplateVariables) {
			templateVariables_ = std::move(pTemplateVariables);
		}
		
		void Function::setSEMFunction(SEM::Function& function) {
			assert(semFunction_ == nullptr);
			assert(&function != nullptr);
			semFunction_ = &function;
		}
		
		SEM::Function& Function::semFunction() {
			assert(semFunction_ != nullptr);
			return *semFunction_;
		}
		
		std::string Function::toString() const {
			if (isDeclaration()) {
				return makeString("FunctionDecl(name = %s, returnType = %s, ... (TODO))",
					name()->toString().c_str(), returnType().toString().c_str());
			} else {
				if (isDefaultDefinition()) {
					return makeString("DefaultFunctionDef(name = %s)",
						name()->toString().c_str());
				} else {
					return makeString("FunctionDef(name = %s, returnType = %s, ... (TODO))",
						name()->toString().c_str(), returnType().toString().c_str());
				}
			}
		}
		
	}
	
}

