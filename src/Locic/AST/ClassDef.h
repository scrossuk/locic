#ifndef LOCIC_AST_CLASSDEF_HPP
#define LOCIC_AST_CLASSDEF_HPP

#include <cstdio>
#include <list>
#include <string>
#include <Locic/AST/FunctionDef.hpp>
#include <Locic/AST/TypeVar.hpp>

namespace AST{

	struct ClassDef{
		std::string name;
		std::list<TypeVar *> memberVariables;
		std::list<FunctionDef *> methodDefinitions;
		
		inline ClassDef(const std::string& n, const std::list<TypeVar *>& m, const std::list<FunctionDecl *>& d)
			: name(n), memberVariables(m),
			methodDefinitions(d){ }
		
		inline void print(){
			printf("class %s(...){\n...\n}\n", name.c_str());
		}
		
	};
	
}

#endif
