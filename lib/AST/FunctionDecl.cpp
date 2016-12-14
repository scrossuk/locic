#include <string>
#include <vector>

#include <locic/Support/Name.hpp>

#include <locic/AST/FunctionDecl.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/Scope.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/Var.hpp>

namespace locic {

	namespace AST {
		
		FunctionDecl::FunctionDecl() :
		isDefinition_(false), isDefaultDefinition_(false), isVarArg_(false),
		isStatic_(false), isImported_(false), isExported_(false),
		isPrimitive_(false), templateVariables_(makeDefaultNode<TemplateVarList>()),
		semFunction_(nullptr) { }
		
		bool FunctionDecl::isDeclaration() const {
			return !isDefinition_;
		}
		
		bool FunctionDecl::isDefinition() const {
			return isDefinition_;
		}
		
		void FunctionDecl::setIsDefinition(const bool value) {
			isDefinition_ = value;
		}
		
		bool FunctionDecl::isDefaultDefinition() const {
			return isDefaultDefinition_;
		}
		
		void FunctionDecl::setIsDefaultDefinition(const bool value) {
			assert(isDefinition());
			isDefaultDefinition_ = value;
		}
		
		bool FunctionDecl::isStatic() const {
			return isStatic_;
		}
		
		void FunctionDecl::setIsStatic(const bool value) {
			isStatic_ = value;
		}
		
		bool FunctionDecl::isVarArg() const {
			return isVarArg_;
		}
		
		void FunctionDecl::setIsVarArg(const bool value) {
			isVarArg_ = value;
		}
		
		bool FunctionDecl::isImported() const {
			return isImported_;
		}
		
		void FunctionDecl::setIsImported(const bool value) {
			isImported_ = value;
		}
		
		bool FunctionDecl::isExported() const {
			return isExported_;
		}
		
		void FunctionDecl::setIsExported(const bool value) {
			isExported_ = value;
		}
		
		bool FunctionDecl::isPrimitive() const {
			return isPrimitive_;
		}
		
		void FunctionDecl::setIsPrimitive(const bool value) {
			isPrimitive_ = value;
		}
		
		const Node<Name>& FunctionDecl::name() const {
			return name_;
		}
		
		void FunctionDecl::setName(Node<Name> pName) {
			name_ = std::move(pName);
		}
		
		Node<TypeDecl>& FunctionDecl::returnType() {
			assert(!isDefaultDefinition());
			return returnType_;
		}
		
		const Node<TypeDecl>& FunctionDecl::returnType() const {
			assert(!isDefaultDefinition());
			return returnType_;
		}
		
		void FunctionDecl::setReturnType(Node<TypeDecl> pReturnType) {
			returnType_ = std::move(pReturnType);
		}
		
		const Node<VarList>& FunctionDecl::parameters() const {
			assert(!isDefaultDefinition());
			return parameters_;
		}
		
		void FunctionDecl::setParameters(Node<VarList> pParameters) {
			parameters_ = std::move(pParameters);
		}
		
		const Node<Scope>& FunctionDecl::scope() const {
			assert(isDefinition() && !isDefaultDefinition());
			return scope_;
		}
		
		void FunctionDecl::setScope(Node<Scope> pScope) {
			assert(isDefinition());
			scope_ = std::move(pScope);
		}
		
		const Node<ConstSpecifier>& FunctionDecl::constSpecifier() const {
			return constSpecifier_;
		}
		
		void FunctionDecl::setConstSpecifier(Node<ConstSpecifier> pConstSpecifier) {
			constSpecifier_ = std::move(pConstSpecifier);
		}
		
		const Node<RequireSpecifier>& FunctionDecl::noexceptSpecifier() const {
			return noexceptSpecifier_;
		}
		
		void FunctionDecl::setNoexceptSpecifier(Node<RequireSpecifier> pNoexceptSpecifier) {
			noexceptSpecifier_ = std::move(pNoexceptSpecifier);
		}
		
		const Node<RequireSpecifier>& FunctionDecl::requireSpecifier() const {
			return requireSpecifier_;
		}
		
		void FunctionDecl::setRequireSpecifier(Node<RequireSpecifier> pRequireSpecifier) {
			requireSpecifier_ = std::move(pRequireSpecifier);
		}
		
		const Node<TemplateVarList>& FunctionDecl::templateVariables() const {
			return templateVariables_;
		}
		
		void FunctionDecl::setTemplateVariables(Node<TemplateVarList> pTemplateVariables) {
			templateVariables_ = std::move(pTemplateVariables);
		}
		
		void FunctionDecl::setSEMFunction(SEM::Function& function) {
			assert(semFunction_ == nullptr);
			semFunction_ = &function;
		}
		
		SEM::Function& FunctionDecl::semFunction() {
			assert(semFunction_ != nullptr);
			return *semFunction_;
		}
		
		std::string FunctionDecl::toString() const {
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

