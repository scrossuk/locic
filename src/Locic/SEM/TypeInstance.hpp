#ifndef LOCIC_SEM_TYPEINSTANCE_HPP
#define LOCIC_SEM_TYPEINSTANCE_HPP

#include <string>
#include <vector>
#include <Locic/SEM/Var.hpp>

namespace SEM{

	struct Function;

	struct TypeInstance{
		enum TypeEnum{
			CLASSDECL = 0,
			CLASSDEF,
			STRUCT
		} typeEnum;
		
		std::string name;
		std::vector<std::string> variableNames;
		std::vector<Var *> variables;
		std::vector<Function *> methods;
		
		inline TypeInstance(TypeEnum e, const std::string& n)
			: typeEnum(e), name(n){ }
		
		inline std::string getFullName() const{
			return name;
		}
	};

}

#endif
