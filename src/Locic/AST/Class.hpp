#ifndef LOCIC_AST_CLASS_HPP
#define LOCIC_AST_CLASS_HPP

#include <cstdio>
#include <list>
#include <string>
#include <Locic/AST/FunctionDecl.hpp>
#include <Locic/AST/FunctionDef.hpp>
#include <Locic/AST/TypeVar.hpp>

namespace AST {

	struct ClassDecl {
		std::string name;
		std::list<FunctionDecl*> methodDeclarations;
		
		inline ClassDecl(const std::string& n, const std::list<FunctionDecl*>& d)
			: name(n), methodDeclarations(d) { }
			
		inline void print() {
			printf("class %s{\n...\n}\n", name.c_str());
		}
		
	};
	
	struct ClassDef {
		std::string name;
		std::list<TypeVar*> memberVariables;
		std::list<FunctionDef*> methodDefinitions;
		
		inline ClassDef(const std::string& n, const std::list<TypeVar*>& m, const std::list<FunctionDecl*>& d)
			: name(n), memberVariables(m),
			  methodDefinitions(d) { }
			  
		inline void print() {
			printf("class %s(...){\n...\n}\n", name.c_str());
		}
		
	};
	
}

#endif
