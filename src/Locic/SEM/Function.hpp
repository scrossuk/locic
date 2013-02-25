#ifndef LOCIC_SEM_FUNCTION_HPP
#define LOCIC_SEM_FUNCTION_HPP

#include <list>
#include <string>
#include <Locic/Map.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace Locic {

	namespace SEM {
	
		struct Scope;
		
		struct Function {
			bool isMethod;
			TypeInstance* parentType;
			Type* type;
			Locic::Name name;
			std::vector<Var*> parameters;
			
			// NULL for declarations.
			Scope* scope;
			
			inline Function(bool isM, Type* t, const Locic::Name& n, const std::vector<Var*>& p, Scope* s, TypeInstance* pT)
				: isMethod(isM), parentType(pT),
				  type(t), name(n),
				  parameters(p), scope(s) { }
				  
			inline static Function* Decl(bool isMethod, TypeInstance* parentType, Type* type, const Locic::Name& name, const std::vector<Var*>& parameters) {
				return new Function(isMethod, type, name, parameters, NULL, parentType);
			}
			
			inline std::string toString() const {
				return makeString("Function(name: %s, isMethod: %s, parent: %s, type: %s)",
						name.toString().c_str(),
						isMethod ? "Yes" : "No",
						parentType != NULL ? parentType->toString().c_str() : "[NONE]",
						type->toString().c_str());
			}
		};
		
	}
	
}

#endif
