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
				const Node<Type>& returnType, const Node<Name>& name,
				const Node<TypeVarList>& parameters,
				const Node<ConstSpecifier>& constSpecifier,
				const Node<RequireSpecifier>& noexceptSpecifier,
				const Node<RequireSpecifier>& requireSpecifier) {
			Function* function = new Function(name);
			function->isDefinition_ = false;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = isVarArg;
			function->isStatic_ = isStatic;
			function->returnType_ = returnType;
			function->parameters_ = parameters;
			function->scope_ = Node<Scope>();
			function->constSpecifier_ = constSpecifier;
			function->noexceptSpecifier_ = noexceptSpecifier;
			function->requireSpecifier_ = requireSpecifier;
			return function;
		}
		
		Function* Function::Def(bool isVarArg, bool isStatic,
				const Node<Type>& returnType, const Node<Name>& name,
				const Node<TypeVarList>& parameters,
				const Node<Scope>& scope,
				const Node<ConstSpecifier>& constSpecifier,
				const Node<RequireSpecifier>& noexceptSpecifier,
				const Node<RequireSpecifier>& requireSpecifier) {
			Function* function = new Function(name);
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = isVarArg;
			function->isStatic_ = isStatic;
			function->returnType_ = returnType;
			function->parameters_ = parameters;
			function->scope_ = scope;
			function->constSpecifier_ = constSpecifier;
			function->noexceptSpecifier_ = noexceptSpecifier;
			function->requireSpecifier_ = requireSpecifier;
			return function;
		}
		
		Function* Function::StaticDecl(const Node<Type>& returnType,
		                               const Node<Name>& name,
		                               const Node<TypeVarList>& parameters,
		                               const Node<RequireSpecifier>& noexceptSpecifier,
		                               const Node<RequireSpecifier>& requireSpecifier) {
			Function* function = new Function(name);
			function->isDefinition_ = false;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = false;
			function->isStatic_ = true;
			function->returnType_ = returnType;
			function->parameters_ = parameters;
			function->noexceptSpecifier_ = noexceptSpecifier;
			function->requireSpecifier_ = requireSpecifier;
			return function;
		}
		
		Function* Function::StaticDef(const Node<Type>& returnType,
		                              const Node<Name>& name,
		                              const Node<TypeVarList>& parameters,
		                              const Node<Scope>& scope,
		                              const Node<RequireSpecifier>& noexceptSpecifier,
		                              const Node<RequireSpecifier>& requireSpecifier) {
			Function* function = new Function(name);
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = false;
			function->isStatic_ = true;
			function->returnType_ = returnType;
			function->parameters_ = parameters;
			function->scope_ = scope;
			function->noexceptSpecifier_ = noexceptSpecifier;
			function->requireSpecifier_ = requireSpecifier;
			return function;
		}
		
		Function* Function::DefaultStaticMethodDef(const Node<Name>& name,
				const Node<RequireSpecifier>& requireSpecifier) {
			Function* function = new Function(name);
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = true;
			function->isVarArg_ = false;
			function->isStatic_ = true;
			function->returnType_ = Node<Type>();
			function->parameters_ = Node<TypeVarList>();
			function->scope_ = Node<Scope>();
			function->requireSpecifier_ = requireSpecifier;
			return function;
		}
		
		Function* Function::DefaultMethodDef(const Node<Name>& name,
				const Node<RequireSpecifier>& requireSpecifier) {
			Function* function = new Function(name);
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = true;
			function->isVarArg_ = false;
			function->isStatic_ = false;
			function->returnType_ = Node<Type>();
			function->parameters_ = Node<TypeVarList>();
			function->scope_ = Node<Scope>();
			function->requireSpecifier_ = requireSpecifier;
			return function;
		}
		
		Function* Function::Destructor(const Node<Name>& name, const Node<Scope>& scope) {
			Function* function = new Function(name);
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = false;
			function->isStatic_ = false;
			function->returnType_ = makeNode(scope.location(), Type::Void());
			function->parameters_ = makeDefaultNode<TypeVarList>();
			function->scope_ = scope;
			return function;
		}
		
		Function::Function(const Node<Name>& pName) :
			isDefinition_(false), isDefaultDefinition_(false),
			isVarArg_(false), isStatic_(false),
			isImported_(false), isExported_(false),
			isPrimitive_(false),
			name_(pName), templateVariables_(makeDefaultNode<TemplateTypeVarList>()),
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
		
		void Function::setTemplateVariables(const Node<TemplateTypeVarList>& pTemplateVariables) {
			templateVariables_ = pTemplateVariables;
		}
		
		void Function::setRequireSpecifier(const Node<RequireSpecifier>& pRequireSpecifier) {
			if (pRequireSpecifier->isNone()) {
				return;
			}
			requireSpecifier_ = pRequireSpecifier;
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

