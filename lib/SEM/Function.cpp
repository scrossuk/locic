#include <locic/String.hpp>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Function* Function::Decl(bool isMethod, bool isStatic, bool isConst, Type* type,
				const Name& name, const std::vector<Var*>& parameters) {
			return new Function(isMethod, isStatic, isConst, type, name, parameters, NULL);
		}
		
		Function* Function::Def(bool isMethod, bool isStatic, bool isConst, Type* type,
				const Name& name, const std::vector<Var*>& parameters, Scope* scope) {
			return new Function(isMethod, isStatic, isConst, type, name, parameters, scope);
		}
		
		Function::Function(bool isM, bool isS, bool isC, Type* t, const Name& n, const std::vector<Var*>& p, Scope* s)
			: isMethod_(isM),
			  isStatic_(isS),
			  isConst_(isC),
			  type_(t), name_(n),
			  parameters_(p), scope_(s) {
			assert(type_ != nullptr);
		}
		
		const Name& Function::name() const {
			return name_;
		}
		
		Type* Function::type() const {
			return type_;
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
		
		const Scope& Function::scope() const {
			assert(isDefinition());
			return *scope_;
		}
		
		Function* Function::createDecl() const {
			return Decl(isMethod(), isStaticMethod(), isConstMethod(), type(), name(), parameters());
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
						declName, substitutedParam);
		}
		
		void Function::setScope(Scope* newScope) {
			assert(scope_ == NULL);
			scope_ = newScope;
			assert(scope_ != NULL);
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

