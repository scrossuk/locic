#ifndef LOCIC_SEM_MODULEGROUP_H
#define LOCIC_SEM_MODULEGROUP_H

#include <Locic/List.h>

typedef struct SEM_ModuleGroup{
	Locic_List * modules;
} SEM_ModuleGroup;

SEM_ModuleGroup * SEM_MakeModuleGroup();

#endif
