#include <locic/String.hpp>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Function* Function::Decl(bool isMethod, bool isStatic, bool isConst, Type* type,
				const Name& name, const std::vector<Var*>& parameters, ModuleScope* moduleScope) {
			return new Function(isMethod, isStatic, isConst, type, name, parameters, moduleScope, nullptr);
		}
		
		Function::Function(bool isM, bool isS, bool isC, Type* t, const Name& n, const std::vector<Var*>& p, ModuleScope* m, Scope* s)
			: isMethod_(isM),
			  isStatic_(isS),
			  isConst_(isC),
			  type_(t), name_(n),
			  parameters_(p),
			  moduleScope_(m),
			  scope_(s) {
			assert(type_ != nullptr);
		}
		
		const Name& Function::name() const {
			return name_;
		}
		
		Type* Function::type() const {
			return type_;
		}
		
		ModuleScope* Function::moduleScope() const {
			return moduleScope_;
		}
		
		bool Function::isDeclaration() const {
			return !isDefinition();
		}
		
		bool Function::isDefinition() const {
			return scope_ != nullptr;
		}
		
		bool Function::isMethod() const {
			return isMethod_;
		}
		
		bool Function::isStaticMethod() const {
			return isStatic_;
		}
		
		bool Function::isConstMethod() const {
			return isConst_;
		}
		
		const std::vector<Var*>& Function::parameters() const {
			return parameters_;
		}
		
		std::map<std::string, Var*>& Function::namedVariables() {
			return namedVariables_;
		}
		
		const std::map<std::string, Var*>& Function::namedVariables() const {
			return namedVariables_;
		}
		
		const Scope& Function::scope() const {
			assert(isDefinition());
			return *scope_;
		}
		
		Function* Function::createDecl() const {
			return Decl(isMethod(), isStaticMethod(), isConstMethod(), type(), name(), parameters(), moduleScope());
		}
		
		Function* Function::fullSubstitute(const Name& declName, const Map<TemplateVar*, Type*>& templateVarMap) const {
			assert(isDeclaration());
			
			// Parameter types need to be substituted.
			std::vector<SEM::Var*> substitutedParam;
			
			for (const auto param: parameters()) {
				substitutedParam.push_back(param->substitute(templateVarMap));
			}
			
			return Decl(isMethod(), isStaticMethod(), isConstMethod(),
						type()->substitute(templateVarMap),
						declName, substitutedParam, moduleScope());
		}
		
		void Function::setScope(Scope* newScope) {
			assert(scope_ == nullptr);
			scope_ = newScope;
			assert(scope_ != nullptr);
		}
		
		std::string Function::toString() const {
			if (isDeclaration()) {
				return makeString("FunctionDeclaration(name: %s, isMethod: %s, isStatic: %s, isConst: %s, type: %s)",
								  name().toString().c_str(),
								  isMethod() ? "Yes" : "No",
								  isStaticMethod() ? "Yes" : "No",
								  isConstMethod() ? "Yes" : "No",
								  type()->toString().c_str());
			} else {
				return makeString("FunctionDefinition(name: %s, isMethod: %s, isStatic: %s, isConst: %s, type: %s, scope: %s)",
								  name().toString().c_str(),
								  isMethod() ? "Yes" : "No",
								  isStaticMethod() ? "Yes" : "No",
								  isConstMethod() ? "Yes" : "No",
								  type()->toString().c_str(),
								  scope().toString().c_str());
			}
		}
		
	}
	
}

