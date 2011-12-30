#include <stdlib.h>
#include <Locic/AST/Function.h>
#include <Locic/List.h>
#include <Locic/AST/Scope.h>
#include <Locic/AST/Type.h>

AST_FunctionDecl * AST_MakeFunctionDecl(AST_Type * returnType, char * name, Locic_List * parameters){
	AST_FunctionDecl * functionDecl = malloc(sizeof(AST_FunctionDecl));
	functionDecl->returnType = returnType;
	functionDecl->name = name;
	functionDecl->parameters = parameters;
	return functionDecl;
}

AST_FunctionDef * AST_MakeFunctionDef(AST_FunctionDecl * declaration, AST_Scope * scope){
	AST_FunctionDef * functionDef = malloc(sizeof(AST_FunctionDef));
	functionDef->declaration = declaration;
	functionDef->scope = scope;
	return functionDef;
}

