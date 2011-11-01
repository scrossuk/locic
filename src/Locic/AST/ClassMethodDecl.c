#include <stdlib.h>
#include <Locic/AST/ClassDecl.h>
#include <Locic/AST/ClassMethodDecl.h>
#include <Locic/AST/List.h>
#include <Locic/AST/Type.h>

AST_ClassMethodDecl * AST_MakeClassMethodDecl(AST_Type * returnType, char * name, AST_List * parameters){
	AST_ClassMethodDecl * classDecl = malloc(sizeof(AST_ClassDecl));
	classDecl->returnType = returnType;
	classDecl->name = name;
	classDecl->parameters = parameters;
	return classDecl;
}

