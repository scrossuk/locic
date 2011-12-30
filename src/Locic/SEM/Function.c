#include <stdlib.h>
#include <Locic/SEM/Function.h>
#include <Locic/List.h>
#include <Locic/SEM/Scope.h>
#include <Locic/SEM/Type.h>

SEM_FunctionDecl * SEM_MakeFunctionDecl(SEM_Type * returnType, char * name, Locic_List * parameters){
	SEM_FunctionDecl * functionDecl = malloc(sizeof(SEM_FunctionDecl));
	functionDecl->returnType = returnType;
	functionDecl->name = name;
	functionDecl->parameters = parameters;
	return functionDecl;
}

SEM_FunctionDef * SEM_MakeFunctionDef(SEM_FunctionDecl * declaration, SEM_Scope * scope){
	SEM_FunctionDef * functionDef = malloc(sizeof(SEM_FunctionDef));
	functionDef->declaration = declaration;
	functionDef->scope = scope;
	return functionDef;
}

