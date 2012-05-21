#ifndef LOCIC_AST_CLASSDECL_HPP
#define LOCIC_AST_CLASSDECL_HPP

#include <cstdio>
#include <list>
#include <string>
#include <Locic/AST/FunctionDecl.hpp>

namespace AST{

	struct ClassDecl{
		std::string name;
		std::list<FunctionDecl *> methodDeclarations;
		
		inline ClassDecl(const std::string& n, const std::list<FunctionDecl *>& d)
			: name(n), methodDeclarations(d){ }
		
		inline void print(){
			printf("class %s{\n...\n}\n", name.c_str());
		}
		
	};
	
}

#endif
