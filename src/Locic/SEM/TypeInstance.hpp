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
			STRUCTDECL,
			STRUCTDEF,
			CLASSDECL,
			CLASSDEF,
			INTERFACE
		} typeEnum;
		
		Locic::Name name;
		Locic::StringMap<Var *> variables;
		Locic::StringMap<Function *> functions;
		
		inline TypeInstance(TypeEnum e, const Locic::Name& n)
			: typeEnum(e), name(n){ }
		
		inline bool isPrimitive() const{
			return typeEnum == PRIMITIVE;
		}
		
		inline bool isStructDecl() const{
			return typeEnum == STRUCTDECL;
		}
		
		inline bool isStructDef() const{
			return typeEnum == STRUCTDEF;
		}
		
		inline bool isStruct() const{
			return isStructDecl() || isStructDef();
		}
		
		inline bool isClassDecl() const{
			return typeEnum == CLASSDECL;
		}
		
		inline bool isClassDef() const{
			return typeEnum == CLASSDEF;
		}
		
		inline bool isClass() const{
			return isClassDecl() || isClassDef();
		}
		
		inline bool isInterface() const{
			return typeEnum == INTERFACE;
		}
		
		inline bool isDeclaration() const{
			return isStructDecl() || isClassDecl();
		}
		
		inline bool isDefinition() const{
			return isStructDef() || isClassDef();
		}
		
		NamespaceNode lookup(const Locic::Name& targetName);
		
		bool supportsNullConstruction() const;
		
		bool supportsImplicitCopy() const;
		
	};

}

#endif
