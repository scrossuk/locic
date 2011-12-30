#ifndef LOCIC_SEM_CLASSDECL_H
#define LOCIC_SEM_CLASSDECL_H

#include <Locic/List.h>

typedef struct SEM_ClassDecl{
	char * name;
	SEM_ClassDef * definition; // NULL if no definition.
	Locic_List * methodDeclarations;
} SEM_ClassDecl;

SEM_ClassDecl * SEM_MakeClassDecl(char * name, Locic_List * declarations);

void SEM_PrintClassDecl(SEM_ClassDecl * decl);

#endif
