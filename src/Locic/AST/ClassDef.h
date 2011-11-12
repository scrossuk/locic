#ifndef LOCIC_AST_CLASSDEF_H
#define LOCIC_AST_CLASSDEF_H

#include <Locic/AST/List.h>

typedef struct AST_ClassDef{
	char * name;
	AST_List * memberVariables;
	AST_List * methodDefinitions;
} AST_ClassDef;

AST_ClassDef * AST_MakeClassDef(char * name, AST_List * members, AST_List * definitions);

void AST_PrintClassDef(AST_ClassDef * def);

#endif
