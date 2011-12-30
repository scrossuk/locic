#ifndef LOCIC_SEM_FUNCTION_H
#define LOCIC_SEM_FUNCTION_H

#include <Locic/List.h>
#include <Locic/SEM/Scope.h>
#include <Locic/SEM/Type.h>

typedef struct SEM_FunctionDecl{
	SEM_Type * returnType;
	char * name;
	Locic_List * parameterVars;
	struct SEM_FunctionDef * definition; // NULL if no definition.
} SEM_FunctionDecl;

SEM_FunctionDecl * SEM_MakeFunctionDecl(SEM_Type * returnType, char * name);

typedef struct SEM_FunctionDef{
	SEM_FunctionDecl * declaration;
	SEM_Scope * scope;
} SEM_FunctionDef;

SEM_FunctionDef * SEM_MakeFunctionDef(SEM_FunctionDecl * declaration, SEM_Scope * scope);

#endif
