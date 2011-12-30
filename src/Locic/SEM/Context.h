#ifndef LOCIC_SEM_CONTEXT_H
#define LOCIC_SEM_CONTEXT_H

#include <Locic/List.h>
#include <Locic/SEM/ClassDecl.h>
#include <Locic/SEM/ClassDef.h>
#include <Locic/SEM/Function.h>

typedef struct SEM_Context{
	Locic_List * modules;
} SEM_Context;

#endif
