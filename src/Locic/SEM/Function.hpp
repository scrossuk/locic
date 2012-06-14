#ifndef LOCIC_SEM_FUNCTION_HPP
#define LOCIC_SEM_FUNCTION_HPP

#include <list>
#include <string>
#include <Locic/SEM/Scope.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>
#include <Locic/SEM/TypeVar.hpp>

namespace SEM {

	struct FunctionDecl {
		TypeInstance * parentType;
		Type* returnType;
		std::string name;
		std::list<Var*> parameters;
		
		inline FunctionDecl(TypeInstance * p, Type* t, const std::string& n, const std::list<TypeVar*>& p)
			: parentType(p), returnType(t), name(n), parameters(p) { }
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
