#ifndef LOCIC_SEM_MODULE_H
#define LOCIC_SEM_MODULE_H

#include <Locic/List.h>
#include <Locic/StringMap.h>
#include <Locic/SEM/Function.h>

typedef struct SEM_Module{
	char * name;
	Locic_StringMap typeInstances;
	Locic_StringMap functionDeclarations;
	Locic_List * functionDefinitions;
} SEM_Module;

SEM_Module * SEM_MakeModule(char * name);

SEM_Module * SEM_ModuleAddFunctionDef(SEM_Module * module, SEM_FunctionDef * functionDef);

void SEM_PrintModule(SEM_Module * module);

#endif
