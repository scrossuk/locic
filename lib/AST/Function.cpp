#include <string>
#include <vector>

#include <locic/AST/Function.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/Scope.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeVar.hpp>

namespace locic {

	namespace AST {
	
		Function* Function::Decl(bool isVarArg, bool isNoExcept, const Node<Type>& returnType, const std::string& name, const Node<TypeVarList>& parameters) {
			Function* function = new Function(name);
			function->isDefinition_ = false;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = isVarArg;
			function->isMethod_ = false;
			function->isConstMethod_ = false;
			function->isStaticMethod_ = false;
			function->isNoExcept_ = isNoExcept;
			function->returnType_ = returnType;
			function->parameters_ = parameters;
			function->scope_ = Node<Scope>();
			return function;
		}
		
		Function* Function::Def(bool isNoExcept, const Node<Type>& returnType, const std::string& name, const Node<TypeVarList>& parameters, const Node<Scope>& scope) {
			Function* function = new Function(name);
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = false;
			function->isMethod_ = false;
			function->isConstMethod_ = false;
			function->isStaticMethod_ = false;
			function->isNoExcept_ = isNoExcept;
			function->returnType_ = returnType;
			function->parameters_ = parameters;
			function->scope_ = scope;
			return function;
		}
		
		Function* Function::StaticMethodDecl(bool isNoExcept, const Node<Type>& returnType, const std::string& name, const Node<TypeVarList>& parameters) {
			Function* function = new Function(name);
			function->isDefinition_ = false;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = false;
			function->isMethod_ = true;
			function->isConstMethod_ = false;
			function->isStaticMethod_ = true;
			function->isNoExcept_ = isNoExcept;
			function->returnType_ = returnType;
			function->parameters_ = parameters;
			function->scope_ = Node<Scope>();
			return function;
		}
		
		Function* Function::MethodDecl(bool isConstMethod, bool isNoExcept, const Node<Type>& returnType, const std::string& name, const Node<TypeVarList>& parameters) {
			Function* function = new Function(name);
			function->isDefinition_ = false;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = false;
			function->isMethod_ = true;
			function->isConstMethod_ = isConstMethod;
			function->isStaticMethod_ = false;
			function->isNoExcept_ = isNoExcept;
			function->returnType_ = returnType;
			function->parameters_ = parameters;
			function->scope_ = Node<Scope>();
			return function;
		}
		
		Function* Function::DefaultStaticMethodDef(const std::string& name) {
			Function* function = new Function(name);
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = true;
			function->isVarArg_ = false;
			function->isMethod_ = true;
			function->isConstMethod_ = false;
			function->isStaticMethod_ = true;
			function->isNoExcept_ = false;
			function->returnType_ = Node<Type>();
			function->parameters_ = Node<TypeVarList>();
			function->scope_ = Node<Scope>();
			return function;
		}
		
		Function* Function::DefaultMethodDef(const std::string& name) {
			Function* function = new Function(name);
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = true;
			function->isVarArg_ = false;
			function->isMethod_ = true;
			function->isConstMethod_ = false;
			function->isStaticMethod_ = false;
			function->isNoExcept_ = false;
			function->returnType_ = Node<Type>();
			function->parameters_ = Node<TypeVarList>();
			function->scope_ = Node<Scope>();
			return function;
		}
		
		Function* Function::Destructor(const Node<Scope>& scope) {
			Function* function = new Function("__destructor");
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = false;
			function->isMethod_ = true;
			function->isConstMethod_ = false;
			function->isStaticMethod_ = false;
			function->isNoExcept_ = true;
			function->returnType_ = makeNode(scope.location(), Type::Void());
			function->parameters_ = makeDefaultNode<TypeVarList>();
			function->scope_ = scope;
			return function;
		}
		
		Function* Function::StaticMethodDef(bool isNoExcept, const Node<Type>& returnType, const std::string& name, const Node<TypeVarList>& parameters, const Node<Scope>& scope) {
			Function* function = new Function(name);
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = false;
			function->isMethod_ = true;
			function->isConstMethod_ = false;
			function->isStaticMethod_ = true;
			function->isNoExcept_ = isNoExcept;
			function->returnType_ = returnType;
			function->parameters_ = parameters;
			function->scope_ = scope;
			return function;
		}
		
		Function* Function::MethodDef(bool isConstMethod, bool isNoExcept, const Node<Type>& returnType, const std::string& name, const Node<TypeVarList>& parameters, const Node<Scope>& scope) {
			Function* function = new Function(name);
			function->isDefinition_ = true;
			function->isDefaultDefinition_ = false;
			function->isVarArg_ = false;
			function->isMethod_ = true;
			function->isConstMethod_ = isConstMethod;
			function->isStaticMethod_ = false;
			function->isNoExcept_ = isNoExcept;
			function->returnType_ = returnType;
			function->parameters_ = parameters;
			function->scope_ = scope;
			return function;
		}
		
		Function::Function(const std::string& pName) :
			isDefinition_(false), isDefaultDefinition_(false),
			isMethod_(false), isConstMethod_(false),
			isStaticMethod_(false), isNoExcept_(false),
			isImported_(false), isExported_(false),
			name_(pName) { }
		
		bool Function::isDeclaration() const {
			return !isDefinition_;
		}
		
		bool Function::isDefinition() const {
			return isDefinition_;
		}
		
		bool Function::isDefaultDefinition() const {
			return isDefinition() && isDefaultDefinition_;
		}
		
		bool Function::isMethod() const {
			return isMethod_;
		}
		
		bool Function::isConstMethod() const {
			return isMethod() && !isStaticMethod() && isConstMethod_;
		}
		
		bool Function::isStaticMethod() const {
			return isMethod() && isStaticMethod_;
		}
		
		bool Function::isVarArg() const {
			return !isMethod() && isVarArg_;
		}
		
		bool Function::isNoExcept() const {
			return isNoExcept_;
		}
		
		bool Function::isImported() const {
			return isImported_;
		}
		
		bool Function::isExported() const {
			return isExported_;
		}
		
		const std::string& Function::name() const {
			return name_;
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
		
		void Function::setImport() {
			isImported_ = true;
		}
		
		void Function::setExport() {
			isExported_ = true;
		}
		
		std::string Function::toString() const {
			if (isDeclaration()) {
				return makeString("FunctionDecl(name = %s, returnType = %s, ... (TODO))",
					name().c_str(), returnType().toString().c_str());
			} else {
				if (isDefaultDefinition()) {
					return makeString("DefaultFunctionDef(name = %s)", name().c_str());
				} else {
					return makeString("FunctionDef(name = %s, returnType = %s, ... (TODO))",
						name().c_str(), returnType().toString().c_str());
				}
			}
		}
		
	}
	
}

