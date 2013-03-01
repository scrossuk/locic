#ifndef LOCIC_SEM_FUNCTION_HPP
#define LOCIC_SEM_FUNCTION_HPP

#include <list>
#include <string>
#include <Locic/Map.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace Locic {

	namespace SEM {
	
		class Scope;
		
		class Function {
			public:
				inline static Function* Decl(bool isMethod, TypeInstance* parentType,
						Type* type, const Locic::Name& name, const std::vector<Var*>& parameters) {
					return new Function(isMethod, type, name, parameters, NULL, parentType);
				}
				
				inline static Function* Def(bool isMethod, TypeInstance* parentType,
						Type* type, const Locic::Name& name, const std::vector<Var*>& parameters, Scope* scope) {
					return new Function(isMethod, type, name, parameters, scope, parentType);
				}
				
				inline const Locic::Name& name() const {
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
				
				inline bool hasParentType() const {
					return parentType_ != NULL;
				}
				
				inline TypeInstance& parentType() const {
					assert(hasParentType());
					return *parentType_;
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
					return makeString("Function(name: %s, isMethod: %s, parent: %s, type: %s)",
							name_.toString().c_str(),
							isMethod_ ? "Yes" : "No",
							parentType_ != NULL ? parentType_->toString().c_str() : "[NONE]",
							type_->toString().c_str());
				}
				
			private:
				inline Function(bool isM, Type* t, const Locic::Name& n, const std::vector<Var*>& p, Scope* s, TypeInstance* pT)
					: isMethod_(isM), parentType_(pT),
					  type_(t), name_(n),
					  parameters_(p), scope_(s) {
					assert(type_ != NULL);
					assert((!isMethod_ || parentType_ != NULL)
						   && "Can't have method with NULL parent type.");
				}
				
				bool isMethod_;
				TypeInstance* parentType_;
				Type* type_;
				Locic::Name name_;
				std::vector<Var*> parameters_;
				
				// NULL for declarations.
				Scope* scope_;
				
		};
		
	}
	
}

#endif
