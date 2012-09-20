#ifndef LOCIC_SEM_TYPEINSTANCE_HPP
#define LOCIC_SEM_TYPEINSTANCE_HPP

#include <string>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/SEM/Var.hpp>

namespace SEM{

	struct Function;
	struct NamespaceNode;

	struct TypeInstance{
		enum TypeEnum{
			CLASSDECL = 0,
			CLASSDEF,
			STRUCT
		} typeEnum;
		
		Locic::Name name;
		Locic::StringMap<Var *> variables;
		Locic::StringMap<Function *> constructors;
		Locic::StringMap<Function *> methods;
		
		inline TypeInstance(TypeEnum e, const Locic::Name& n)
			: typeEnum(e), name(n){ }
		
		NamespaceNode lookup(const Locic::Name& targetName);
		
	};

}

#endif
