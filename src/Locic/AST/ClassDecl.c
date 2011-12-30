#include <stdio.h>
#include <stdlib.h>
#include <Locic/AST/ClassDecl.h>
#include <Locic/List.h>

AST_ClassDecl * AST_MakeClassDecl(char * name, Locic_List * declarations){
	AST_ClassDecl * classDecl = malloc(sizeof(AST_ClassDecl));
	classDecl->name = name;
	classDecl->methodDeclarations = declarations;
	return classDecl;
}

void AST_PrintClassDecl(AST_ClassDecl * decl){
	printf("class %s{\n...\n}\n", decl->name);
}

