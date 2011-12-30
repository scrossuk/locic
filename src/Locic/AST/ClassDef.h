#ifndef LOCIC_AST_CLASSDEF_H
#define LOCIC_AST_CLASSDEF_H

#include <Locic/List.h>

typedef struct AST_ClassDef{
	char * name;
	Locic_List * memberVariables;
	Locic_List * methodDefinitions;
} AST_ClassDef;

AST_ClassDef * AST_MakeClassDef(char * name, Locic_List * members, Locic_List * definitions);

void AST_PrintClassDef(AST_ClassDef * def);

#endif
