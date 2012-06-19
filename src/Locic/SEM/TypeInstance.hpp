#ifndef LOCIC_SEM_TYPEINSTANCE_HPP
#define LOCIC_SEM_TYPEINSTANCE_HPP

#include <list>
#include <string>
#include <Locic/SEM/Var.hpp>

namespace SEM{

	struct TypeInstance{
		enum TypeEnum{
			CLASSDECL,
			CLASSDEF,
			STRUCT
		} typeEnum;
		
		std::string name;
		TypeInstance * declaration;
		std::list<Var *> variables;
		
		inline TypeInstance(TypeEnum e, const std::string& n, TypeInstance * d)
			: typeEnum(e), name(n), declaration(d){ }
		
		TypeInstance * ClassDecl(const std::string& name){
			return new TypeInstance(CLASSDECL, name, 0);
		}
		
		TypeInstance * ClassDef(TypeInstance * decl){
			return new TypeInstance(CLASSDEF, decl->name, decl);
		}
		
		TypeInstance * Struct(const std::string& name){
			return new TypeInstance(STRUCT, name, 0);
		}
	};

}

#endif
