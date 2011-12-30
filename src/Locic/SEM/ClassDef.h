#ifndef LOCIC_SEM_CLASSDEF_H
#define LOCIC_SEM_CLASSDEF_H

#include <Locic/List.h>
#include <Locic/SEM/ClassDecl.h>

typedef struct SEM_ClassDef{
	SEM_ClassDecl * declaration;
	Locic_List * memberVariables;
	Locic_List * methodDefinitions;
} SEM_ClassDef;

SEM_ClassDef * SEM_MakeClassDef(char * name, Locic_List * members, Locic_List * definitions);

void SEM_PrintClassDef(SEM_ClassDef * def);

#endif
