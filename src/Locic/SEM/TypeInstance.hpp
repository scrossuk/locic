#ifndef LOCIC_SEM_TYPEINSTANCE_HPP
#define LOCIC_SEM_TYPEINSTANCE_HPP

#include <string>
#include <vector>
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
		std::vector<std::string> variableNames;
		std::vector<Var *> variables;
		
		inline TypeInstance(TypeEnum e, const std::string& n, TypeInstance * d)
			: typeEnum(e), name(n), declaration(d){ }
		
		inline static TypeInstance * ClassDecl(const std::string& name){
			return new TypeInstance(CLASSDECL, name, NULL);
		}
		
		inline static TypeInstance * ClassDef(TypeInstance * decl){
			return new TypeInstance(CLASSDEF, decl->name, decl);
		}
		
		inline static TypeInstance * Struct(const std::string& name){
			return new TypeInstance(STRUCT, name, NULL);
		}
	};

}

#endif
