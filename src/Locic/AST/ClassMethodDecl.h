#ifndef LOCIC_AST_CLASSMETHODDECL_H
#define LOCIC_AST_CLASSMETHODDECL_H

#include <Locic/AST/List.h>

typedef struct AST_ClassMethodDecl{
	AST_Type * returnType;
	char * name;
	AST_List * parameters;
} AST_ClassMethodDecl;

inline AST_ClassMethodDecl * AST_MakeClassMethodDecl(AST_Type * returnType, char * name, AST_List * parameters){
	AST_ClassMethodDecl * classDecl = malloc(sizeof(AST_ClassDecl));
	classDecl->returnType = returnType;
	classDecl->name = name;
	classDecl->parameters = parameters;
	return classDecl;
}

#endif
