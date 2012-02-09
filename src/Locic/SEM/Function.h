#ifndef LOCIC_SEM_FUNCTION_H
#define LOCIC_SEM_FUNCTION_H

#include <Locic/List.h>
#include <Locic/SEM/Scope.h>
#include <Locic/SEM/Type.h>
#include <Locic/SEM/TypeInstance.h>

typedef struct SEM_FunctionDecl{
	SEM_TypeInstance * parentType;
	SEM_Type * type;
	char * name;
	Locic_List * parameterVars;
} SEM_FunctionDecl;

SEM_FunctionDecl * SEM_MakeFunctionDecl(SEM_TypeInstance * parentType, SEM_Type * type, char * name, Locic_List * parameterVars);

typedef struct SEM_FunctionDef{
	SEM_TypeInstance * parentType;
	SEM_FunctionDecl * declaration;
	SEM_Scope * scope;
} SEM_FunctionDef;

SEM_FunctionDef * SEM_MakeFunctionDef(SEM_TypeInstance * parentType, SEM_FunctionDecl * declaration, SEM_Scope * scope);

#endif
