#ifndef LOCIC_SEM_FUNCTION_HPP
#define LOCIC_SEM_FUNCTION_HPP

#include <list>
#include <string>
#include <Locic/Map.hpp>

#include <Locic/SEM/Object.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace Locic {

	namespace SEM {
	
		class Scope;
		
		class Function: public Object {
			public:
				inline static Function* Decl(bool isMethod, bool isStatic, Type* type,
					const std::string& name, const std::vector<Var*>& parameters) {
					return new Function(isMethod, isStatic, type, name, parameters, NULL);
				}
				
				inline static Function* Def(bool isMethod, bool isStatic, Type* type,
					const std::string& name, const std::vector<Var*>& parameters, Scope* scope) {
					return new Function(isMethod, isStatic, type, name, parameters, scope);
				}
				
				inline Function* createDecl() const {
					return Decl(isMethod(), isStatic(), type(), name(), parameters());
				}
				
				inline Function* fullSubstitute(const Map<TemplateVar*, Type*>& templateVarMap) const {
					assert(isDeclaration());
					
					// Parameter types need to be substituted.
					std::vector<SEM::Var*> substitutedParam;
					for(size_t i = 0; i < parameters().size(); i++){
						assert(parameters().at(i)->kind() == Var::PARAM);
						substitutedParam.push_back(Var::Param(
							parameters().at(i)->type()->substitute(templateVarMap)));
					}
					
					return Decl(isMethod(), isStatic(),
						type()->substitute(templateVarMap),
						name(), substitutedParam);
				}
				
				inline ObjectKind objectKind() const {
					return OBJECT_FUNCTION;
				}
				
				inline const std::string& name() const {
					return name_;
				}
				
				inline Type* type() const {
					return type_;
				}
				
				inline bool isDeclaration() const {
					return scope_ == NULL;
				}
				
				inline bool isDefinition() const {
					return scope_ != NULL;
				}
				
				inline bool isMethod() const {
					return isMethod_;
				}
				
				inline bool isStatic() const {
					return isStatic_;
				}
				
				inline const std::vector<Var*>& parameters() const {
					return parameters_;
				}
				
				inline const Scope& scope() const {
					assert(isDefinition());
					return *scope_;
				}
				
				inline void setScope(Scope* newScope) {
					assert(scope_ == NULL);
					scope_ = newScope;
					assert(scope_ != NULL);
				}
				
				inline std::string toString() const {
					return makeString("Function(name: %s, isMethod: %s, isStatic: %s, type: %s)",
							name().c_str(),
							isMethod() ? "Yes" : "No",
							isStatic() ? "Yes" : "No",
							type()->toString().c_str());
				}
				
			private:
				inline Function(bool isM, bool isS, Type* t, const std::string& n, const std::vector<Var*>& p, Scope* s)
					: isMethod_(isM),
					  isStatic_(isS),
					  type_(t), name_(n),
					  parameters_(p), scope_(s) {
					assert(type_ != NULL);
				}
				
				bool isMethod_, isStatic_;
				Type* type_;
				std::string name_;
				std::vector<Var*> parameters_;
				
				// NULL for declarations.
				Scope* scope_;
				
		};
		
	}
	
}

#endif
