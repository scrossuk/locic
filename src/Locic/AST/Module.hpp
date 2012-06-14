#ifndef LOCIC_AST_MODULE_HPP
#define LOCIC_AST_MODULE_HPP

#include <list>
#include <string>

#include <Locic/AST/Class.hpp>
#include <Locic/AST/Function.hpp>
#include <Locic/AST/Struct.hpp>

namespace AST {

	struct Module {
		std::string name;
		std::list<Struct *> structs;
		std::list<FunctionDecl *> functionDeclarations;
		std::list<FunctionDef *> functionDefinitions;
		std::list<ClassDecl *> classDeclarations;
		std::list<ClassDef *> classDefinitions;
		
		inline Module(const std::string& n)
			: name(n) { }
			
		inline void print() {
			// TODO
		}
	};
	
}

#endif
