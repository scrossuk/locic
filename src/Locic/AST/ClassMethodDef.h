#ifndef LOCIC_AST_CLASSMETHODDEF_H
#define LOCIC_AST_CLASSMETHODDEF_H

#include <Locic/AST/ClassMethodDecl.h>
#include <Locic/AST/Scope.h>

typedef struct AST_ClassMethodDef{
	AST_ClassMethodDecl * declaration;
	AST_Scope * scope;
} AST_ClassMethodDef;

AST_ClassMethodDef * AST_MakeClassMethodDef(AST_ClassMethodDecl * declaration, AST_Scope * scope);

#endif
