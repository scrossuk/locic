#include <stdlib.h>
#include <Locic/AST/ClassDecl.h>
#include <Locic/AST/List.h>

AST_ClassDecl * AST_MakeClassDecl(char * name, AST_List * declarations){
	AST_ClassDecl * classDecl = malloc(sizeof(AST_ClassDecl));
	classDecl->name = name;
	classDecl->methodDeclarations = declarations;
	return classDecl;
}

