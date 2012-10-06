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
			PRIMITIVE,
			STRUCT,
			CLASSDECL,
			CLASSDEF
		} typeEnum;
		
		Locic::Name name;
		Locic::StringMap<Var *> variables;
		Locic::StringMap<Function *> functions;
		
		inline TypeInstance(TypeEnum e, const Locic::Name& n)
			: typeEnum(e), name(n){ }
		
		NamespaceNode lookup(const Locic::Name& targetName);
		
		bool supportsImplicitCopy() const;
		
		bool isClass() const;
		
	};

}

#endif
