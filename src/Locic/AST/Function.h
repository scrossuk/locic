#ifndef LOCIC_AST_FUNCTION_H
#define LOCIC_AST_FUNCTION_H

#include <list>
#include <string>
#include <Locic/AST/Scope.hpp>
#include <Locic/AST/Type.hpp>
#include <Locic/AST/TypeVar.hpp>

namespace AST{

	struct FunctionDecl{
		Type * returnType;
		std::string name;
		std::list<TypeVar *> parameters;
		
		inline FunctionDecl(Type * t, const std::string& n, const std::list<TypeVar *>& p)
			: returnType(t), name(n), parameters(p){ }
	};
	
	struct FunctionDef{
		FunctionDecl * declaration;
		Scope * scope;
		
		inline FunctionDef(FunctionDecl * d, Scope * s)
			: declaration(d), scope(s){ }
	};

}

#endif
