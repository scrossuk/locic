#include <stdlib.h>
#include <stdio.h>
#include <Locic/List.h>
#include <Locic/StringMap.h>
#include <Locic/SEM/Module.h>

SEM_Module * SEM_MakeModule(char * name){
	SEM_Module * module = malloc(sizeof(SEM_Module));
	module->name = name;
	module->typeInstances = Locic_StringMap_Alloc();
	module->functionDeclarations = Locic_StringMap_Alloc();
	module->functionDefinitions = Locic_List_Alloc();
	return module;
}

SEM_Module * SEM_ModuleAddFunctionDef(SEM_Module * module, SEM_FunctionDef * functionDef){
	module->functionDefinitions = Locic_List_Append(module->functionDefinitions, functionDef);
	return module;
}

