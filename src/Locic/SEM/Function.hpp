#ifndef LOCIC_SEM_FUNCTION_HPP
#define LOCIC_SEM_FUNCTION_HPP

#include <list>
#include <string>
#include <Locic/Map.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace SEM {

	struct Scope;

	struct Function{
		TypeInstance * parentType;
		Type * type;
		Locic::Name name;
		std::vector<Var *> parameters;
		
		// NULL for declarations.
		Scope * scope;
		
		inline Function(Type * t, const Locic::Name& n, const std::vector<Var*>& p, Scope * s, TypeInstance * pT)
			: parentType(pT),
			type(t), name(n),
			parameters(p), scope(s) { }
			
		inline static Function * Decl(TypeInstance * parentType, Type * type, const Locic::Name& name, const std::vector<Var*>& parameters){
			return new Function(type, name, parameters, NULL, parentType);
		}
		
		inline static Function * Def(TypeInstance * parentType, Type * type, const Locic::Name& name, const std::vector<Var*>& parameters, Scope * scope){
			return new Function(type, name, parameters, scope, parentType);
		}
	};
	
}

#endif
