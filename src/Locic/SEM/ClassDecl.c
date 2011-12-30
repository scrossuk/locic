#include <stdio.h>
#include <stdlib.h>
#include <Locic/SEM/ClassDecl.h>
#include <Locic/List.h>

SEM_ClassDecl * SEM_MakeClassDecl(char * name, Locic_List * declarations){
	SEM_ClassDecl * classDecl = malloc(sizeof(SEM_ClassDecl));
	classDecl->name = name;
	classDecl->methodDeclarations = declarations;
	classDecl->definition = NULL;
	return classDecl;
}

void SEM_PrintClassDecl(SEM_ClassDecl * decl){
	printf("class %s{\n...\n}\n", decl->name);
}

