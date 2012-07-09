#ifndef LOCIC_SEM_FUNCTION_HPP
#define LOCIC_SEM_FUNCTION_HPP

#include <list>
#include <string>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace SEM {

	struct Scope;

	struct Function{
		enum TypeEnum{
			DEFINITION,
			DECLARATION
		} typeEnum;
		
		TypeInstance * parentType;
		Type * type;
		std::string name;
		std::vector<Var *> parameters;
		
		// NULL for declarations.
		Scope * scope;
		
		inline Function(TypeEnum e, Type * t, const std::string& n, const std::vector<Var*>& p, Scope * s, TypeInstance * pT)
			: parentType(pT), typeEnum(e),
			type(t), name(n),
			parameters(p), scope(s) { }
			
		inline static Function * Decl(TypeInstance * parentType, Type * type, const std::string& name, const std::vector<Var*>& parameters){
			return new Function(DECLARATION, type, name, parameters, NULL, parentType);
		}
		
		inline static Function * Def(TypeInstance * parentType, Type * type, const std::string& name, const std::vector<Var*>& parameters, Scope * scope){
			return new Function(DEFINITION, type, name, parameters, scope, parentType);
		}
		
		inline std::string getFullName() const{
			return parentType != NULL ? (parentType->getFullName() + "__" + name) : name;
		}
	};
	
}

#endif
