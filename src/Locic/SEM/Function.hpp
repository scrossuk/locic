#ifndef LOCIC_SEM_FUNCTION_HPP
#define LOCIC_SEM_FUNCTION_HPP

#include <list>
#include <string>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace SEM {

	struct Scope;

	struct FunctionDecl {
		TypeInstance * parentType;
		Type* type;
		std::string name;
		std::list<Var*> parameters;
		
		inline FunctionDecl(TypeInstance * p, Type* t, const std::string& n, const std::list<Var*>& param)
			: parentType(p), type(t), name(n), parameters(param) { }
	};
	
	struct FunctionDef {
		TypeInstance * parentType;
		FunctionDecl* declaration;
		Scope* scope;
		
		inline FunctionDef(TypeInstance * p, FunctionDecl* d, Scope* s)
			: parentType(p), declaration(d), scope(s) { }
	};
	
}

#endif
