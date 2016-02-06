#include <string>
#include <vector>

#include <locic/Support/Name.hpp>

#include <locic/AST/Function.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/Scope.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeVar.hpp>

namespace locic {

	namespace AST {
	
		Function* Function::Decl(bool isVarArg, bool isStatic,
				Node<Type> returnType, Node<Name> name,
				Node<TypeVarList> parameters,
				Node<ConstSpecifier> constSpecifier,
				Node<RequireSpecifier> noexceptSpecifier,
				Node<RequireSpecifier> requireSpecifier) {
			Function* function = new Function(std::move(name));
			function->isDefinition_ = false;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = isVarArg;
			function->isStatic_ = isStatic;
			function->returnType_ = std::move(returnType);
			function->parameters_ = std::move(parameters);
			function->scope_ = Node<Scope>();
			function->constSpecifier_ = std::move(constSpecifier);
			function->noexceptSpecifier_ = std::move(noexceptSpecifier);
			function->requireSpecifier_ = std::move(requireSpecifier);
			return function;
		}
		
		Function* Function::Def(bool isVarArg, bool isStatic,
				Node<Type> returnType, Node<Name> name,
				Node<TypeVarList> parameters,
				Node<Scope> scope,
				Node<ConstSpecifier> constSpecifier,
				Node<RequireSpecifier> noexceptSpecifier,
				Node<RequireSpecifier> requireSpecifier) {
			Function* function = new Function(std::move(name));
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = isVarArg;
			function->isStatic_ = isStatic;
			function->returnType_ = std::move(returnType);
			function->parameters_ = std::move(parameters);
			function->scope_ = std::move(scope);
			function->constSpecifier_ = std::move(constSpecifier);
			function->noexceptSpecifier_ = std::move(noexceptSpecifier);
			function->requireSpecifier_ = std::move(requireSpecifier);
			return function;
		}
		
		Function* Function::StaticDecl(Node<Type> returnType,
		                               Node<Name> name,
		                               Node<TypeVarList> parameters,
		                               Node<RequireSpecifier> noexceptSpecifier,
		                               Node<RequireSpecifier> requireSpecifier) {
			Function* function = new Function(std::move(name));
			function->isDefinition_ = false;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = false;
			function->isStatic_ = true;
			function->returnType_ = std::move(returnType);
			function->parameters_ = std::move(parameters);
			function->noexceptSpecifier_ = std::move(noexceptSpecifier);
			function->requireSpecifier_ = std::move(requireSpecifier);
			return function;
		}
		
		Function* Function::StaticDef(Node<Type> returnType,
		                              Node<Name> name,
		                              Node<TypeVarList> parameters,
		                              Node<Scope> scope,
		                              Node<RequireSpecifier> noexceptSpecifier,
		                              Node<RequireSpecifier> requireSpecifier) {
			Function* function = new Function(std::move(name));
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = false;
			function->isStatic_ = true;
			function->returnType_ = std::move(returnType);
			function->parameters_ = std::move(parameters);
			function->scope_ = std::move(scope);
			function->noexceptSpecifier_ = std::move(noexceptSpecifier);
			function->requireSpecifier_ = std::move(requireSpecifier);
			return function;
		}
		
		Function* Function::DefaultStaticMethodDef(Node<Name> name,
				Node<RequireSpecifier> requireSpecifier) {
			Function* function = new Function(std::move(name));
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = true;
			function->isVarArg_ = false;
			function->isStatic_ = true;
			function->returnType_ = Node<Type>();
			function->parameters_ = Node<TypeVarList>();
			function->scope_ = Node<Scope>();
			function->requireSpecifier_ = std::move(requireSpecifier);
			return function;
		}
		
		Function* Function::DefaultMethodDef(Node<Name> name,
				Node<RequireSpecifier> requireSpecifier) {
			Function* function = new Function(std::move(name));
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = true;
			function->isVarArg_ = false;
			function->isStatic_ = false;
			function->returnType_ = Node<Type>();
			function->parameters_ = Node<TypeVarList>();
			function->scope_ = Node<Scope>();
			function->requireSpecifier_ = std::move(requireSpecifier);
			return function;
		}
		
		Function* Function::Destructor(Node<Name> name, Node<Scope> scope) {
			Function* function = new Function(std::move(name));
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = false;
			function->isStatic_ = false;
			function->returnType_ = makeNode(scope.location(), Type::Void());
			function->parameters_ = makeDefaultNode<TypeVarList>();
			function->scope_ = std::move(scope);
			return function;
		}
		
		Function::Function(Node<Name> pName) :
			isDefinition_(false), isDefaultDefinition_(false),
			isVarArg_(false), isStatic_(false),
			isImported_(false), isExported_(false),
			isPrimitive_(false), name_(std::move(pName)),
			templateVariables_(makeDefaultNode<TemplateTypeVarList>()),
			semFunction_(nullptr) { }
		
		bool Function::isDeclaration() const {
			return !isDefinition_;
		}
		
		bool Function::isDefinition() const {
			return isDefinition_;
		}
		
		bool Function::isDefaultDefinition() const {
			return isDefinition() && isDefaultDefinition_;
		}
		
		bool Function::isStatic() const {
			return isStatic_;
		}
		
		bool Function::isVarArg() const {
			return isVarArg_;
		}
		
		bool Function::isImported() const {
			return isImported_;
		}
		
		bool Function::isExported() const {
			return isExported_;
		}
		
		bool Function::isPrimitive() const {
			return isPrimitive_;
		}
		
		const Node<Name>& Function::name() const {
			return name_;
		}
		
		const Node<TemplateTypeVarList>& Function::templateVariables() const {
			return templateVariables_;
		}
		
		const Node<Type>& Function::returnType() const {
			assert(!isDefaultDefinition());
			return returnType_;
		}
		
		const Node<TypeVarList>& Function::parameters() const {
			assert(!isDefaultDefinition());
			return parameters_;
		}
		
		const Node<Scope>& Function::scope() const {
			assert(isDefinition() && !isDefaultDefinition());
			return scope_;
		}
		
		const Node<ConstSpecifier>& Function::constSpecifier() const {
			return constSpecifier_;
		}
		
		const Node<RequireSpecifier>& Function::noexceptSpecifier() const {
			return noexceptSpecifier_;
		}
		
		const Node<RequireSpecifier>& Function::requireSpecifier() const {
			return requireSpecifier_;
		}
		
		void Function::setTemplateVariables(Node<TemplateTypeVarList> pTemplateVariables) {
			templateVariables_ = std::move(pTemplateVariables);
		}
		
		void Function::setRequireSpecifier(Node<RequireSpecifier> pRequireSpecifier) {
			if (pRequireSpecifier->isNone()) {
				return;
			}
			requireSpecifier_ = std::move(pRequireSpecifier);
		}
		
		void Function::setImport() {
			isImported_ = true;
		}
		
		void Function::setExport() {
			isExported_ = true;
		}
		
		void Function::setPrimitive() {
			isPrimitive_ = true;
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

