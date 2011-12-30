#ifndef LOCIC_SEM_MODULE_H
#define LOCIC_SEM_MODULE_H

#include <Locic/List.h>
#include <Locic/SEM/ClassDecl.h>
#include <Locic/SEM/ClassDef.h>
#include <Locic/SEM/Function.h>

typedef struct SEM_Module{
	char * name;
	Locic_List * functionDeclarations;
	Locic_List * functionDefinitions;
	Locic_List * classDeclarations;
	Locic_List * classDefinitions;
} SEM_Module;

SEM_Module * SEM_MakeModule(char * name);

SEM_Module * SEM_ModuleAddFunctionDecl(SEM_Module * module, SEM_FunctionDecl * functionDecl);

SEM_Module * SEM_ModuleAddFunctionDef(SEM_Module * module, SEM_FunctionDef * functionDef);

SEM_Module * SEM_ModuleAddClassDecl(SEM_Module * module, SEM_ClassDecl * classDecl);

SEM_Module * SEM_ModuleAddClassDef(SEM_Module * module, SEM_ClassDef * classDef);

void SEM_PrintModule(SEM_Module * module);

#endif
