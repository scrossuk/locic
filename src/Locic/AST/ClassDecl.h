#ifndef LOCIC_AST_CLASSDECL_H
#define LOCIC_AST_CLASSDECL_H

#include <Locic/AST/List.h>

typedef struct AST_ClassDecl{
	char * name;
	AST_List * methodDeclarations;
} AST_ClassDecl;

AST_ClassDecl * AST_MakeClassDecl(char * name, AST_List * declarations);

#endif
