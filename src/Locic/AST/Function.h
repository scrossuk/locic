#ifndef LOCIC_AST_FUNCTION_H
#define LOCIC_AST_FUNCTION_H

#include <Locic/List.h>
#include <Locic/AST/Scope.h>
#include <Locic/AST/Type.h>

typedef struct AST_FunctionDecl{
	AST_Type * returnType;
	char * name;
	Locic_List * parameters;
} AST_FunctionDecl;

AST_FunctionDecl * AST_MakeFunctionDecl(AST_Type * returnType, char * name, Locic_List * parameters);

typedef struct AST_FunctionDef{
	AST_FunctionDecl * declaration;
	AST_Scope * scope;
} AST_FunctionDef;

AST_FunctionDef * AST_MakeFunctionDef(AST_FunctionDecl * declaration, AST_Scope * scope);

#endif
