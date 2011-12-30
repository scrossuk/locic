#ifndef LOCIC_AST_CLASSDECL_H
#define LOCIC_AST_CLASSDECL_H

#include <Locic/List.h>

typedef struct AST_ClassDecl{
	char * name;
	Locic_List * methodDeclarations;
} AST_ClassDecl;

AST_ClassDecl * AST_MakeClassDecl(char * name, Locic_List * declarations);

void AST_PrintClassDecl(AST_ClassDecl * decl);

#endif
