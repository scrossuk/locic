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
				inline static Function* Decl(bool isMethod, TypeInstance* parentType,
						Type* type, const std::string& name, const std::vector<Var*>& parameters) {
					return new Function(isMethod, type, name, parameters, NULL);
				}
				
				inline static Function* Def(bool isMethod, TypeInstance* parentType,
						Type* type, const std::string& name, const std::vector<Var*>& parameters, Scope* scope) {
					return new Function(isMethod, type, name, parameters, scope);
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
					return makeString("Function(name: %s, isMethod: %s, type: %s)",
							name_.c_str(),
							isMethod_ ? "Yes" : "No",
							type_->toString().c_str());
				}
				
			private:
				inline Function(bool isM, Type* t, const std::string& n, const std::vector<Var*>& p, Scope* s)
					: isMethod_(isM),
					  type_(t), name_(n),
					  parameters_(p), scope_(s) {
					assert(type_ != NULL);
				}
				
				bool isMethod_;
				Type* type_;
				std::string name_;
				std::vector<Var*> parameters_;
				
				// NULL for declarations.
				Scope* scope_;
				
		};
		
	}
	
}

#endif
