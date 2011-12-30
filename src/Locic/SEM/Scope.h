#ifndef LOCIC_SEM_SCOPE_H
#define LOCIC_SEM_SCOPE_H

#include <Locic/Array.h>
#include <Locic/List.h>

typedef struct SEM_Scope{
	Locic_Array * localVariables;
	Locic_List * statementList;
} SEM_Scope;

SEM_Scope * SEM_MakeScope();

#endif
