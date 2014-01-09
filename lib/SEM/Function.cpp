#include <locic/String.hpp>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Function* Function::Decl(bool isMethod, bool isStatic, bool isConst, Type* type,
				const Name& name, const std::vector<Var*>& parameters) {
			const bool defaultImplementation = false;
			return new Function(isMethod, isStatic, isConst, defaultImplementation, type, name, parameters, NULL);
		}
		
		Function* Function::Def(bool isMethod, bool isStatic, bool isConst, Type* type,
				const Name& name, const std::vector<Var*>& parameters, Scope* scope) {
			const bool defaultImplementation = false;
			return new Function(isMethod, isStatic, isConst, defaultImplementation, type, name, parameters, scope);
		}
		
		Function* Function::DefDefault(bool isStatic, Type* type, const Name& name) {
			// Only methods can have default implementations.
			const bool isMethod = true;
			
			const bool isConst = false;
			
			const bool defaultImplementation = true;
			
			// No parameters need to be created, since
			// they're only used for generating the
			// implementation of a function.
			const auto parameters = std::vector<Var*>();
			
			return new Function(isMethod, isStatic, isConst, defaultImplementation, type, name, parameters, NULL);	
		}
		
		Function::Function(bool isM, bool isS, bool isC, bool hasD, Type* t, const Name& n, const std::vector<Var*>& p, Scope* s)
			: isMethod_(isM),
			  isStatic_(isS),
			  isConst_(isC),
			  hasDefaultImplementation_(hasD),
			  type_(t), name_(n),
			  parameters_(p), scope_(s) {
			assert(type_ != NULL);
			assert(!(hasDefaultImplementation_ && scope_ != NULL));
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
			return hasDefaultImplementation_ || scope_ != NULL;
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
		
		bool Function::hasDefaultImplementation() const {
			return hasDefaultImplementation_;
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
			} else if (hasDefaultImplementation()) {
				return makeString("FunctionDefaultDefinition(name: %s, isMethod: %s, isStatic: %s, isConst: %s, type: %s)",
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

