#ifndef LOCIC_AST_CLASSMETHODDECL_H
#define LOCIC_AST_CLASSMETHODDECL_H

#include <Locic/AST/List.h>
#include <Locic/AST/Type.h>

typedef struct AST_ClassMethodDecl{
	AST_Type * returnType;
	char * name;
	AST_List * parameters;
} AST_ClassMethodDecl;

AST_ClassMethodDecl * AST_MakeClassMethodDecl(AST_Type * returnType, char * name, AST_List * parameters);

#endif
