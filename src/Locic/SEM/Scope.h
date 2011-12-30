#ifndef LOCIC_SEM_SCOPE_H
#define LOCIC_SEM_SCOPE_H

#include <Locic/List.h>

typedef struct SEM_Scope{
	Locic_List * statementList;
} SEM_Scope;

SEM_Scope * SEM_MakeScope(Locic_List * statementList);

#endif
