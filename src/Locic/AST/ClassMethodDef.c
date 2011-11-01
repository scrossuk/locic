#include <stdlib.h>
#include <Locic/AST/ClassMethodDecl.h>
#include <Locic/AST/ClassMethodDef.h>
#include <Locic/AST/Scope.h>

AST_ClassMethodDef * AST_MakeClassMethodDef(AST_ClassMethodDecl * declaration, AST_Scope * scope){
	AST_ClassMethodDef * classMethodDef = malloc(sizeof(AST_ClassMethodDef));
	classMethodDef->declaration = declaration;
	classMethodDef->scope = scope;
	return classMethodDef;
}

