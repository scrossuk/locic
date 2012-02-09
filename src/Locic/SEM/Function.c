#include <stdlib.h>
#include <Locic/List.h>
#include <Locic/SEM/Function.h>
#include <Locic/SEM/Scope.h>
#include <Locic/SEM/Type.h>

SEM_FunctionDecl * SEM_MakeFunctionDecl(SEM_TypeInstance * parentType, SEM_Type * type, char * name, Locic_List * parameterVars){
	SEM_FunctionDecl * functionDecl = malloc(sizeof(SEM_FunctionDecl));
	functionDecl->parentType = parentType;
	functionDecl->type = type;
	functionDecl->name = name;
	functionDecl->parameterVars = parameterVars;
	return functionDecl;
}

SEM_FunctionDef * SEM_MakeFunctionDef(SEM_TypeInstance * parentType, SEM_FunctionDecl * declaration, SEM_Scope * scope){
	SEM_FunctionDef * functionDef = malloc(sizeof(SEM_FunctionDef));
	functionDef->parentType = parentType;
	functionDef->declaration = declaration;
	functionDef->scope = scope;
	return functionDef;
}

