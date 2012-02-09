#include <assert.h>
#include <Locic/List.h>
#include <Locic/SEM/TypeInstance.h>

SEM_TypeInstance * SEM_MakeClassDecl(char * name){
	SEM_TypeInstance * typeInstance = malloc(sizeof(SEM_TypeInstance));
	typeInstance->typeEnum = SEM_TYPEINST_CLASSDECL;
	typeInstance->declaration = NULL;
	typeInstance->name = name;
	typeInstance->variables = NULL;
	return typeInstance;
}

SEM_TypeInstance * SEM_MakeClassDef(SEM_TypeInstance * declaration){
	assert(declaration != NULL);
	assert(declaration->typeEnum == SEM_TYPEINST_CLASSDECL);

	SEM_TypeInstance * typeInstance = malloc(sizeof(SEM_TypeInstance));
	typeInstance->typeEnum = SEM_TYPEINST_CLASSDEF;
	typeInstance->declaration = declaration;
	typeInstance->name = declaration->name;
	typeInstance->variables = Locic_List_Alloc();
	return typeInstance;
}

SEM_TypeInstance * SEM_MakeStruct(char * name){
	SEM_TypeInstance * typeInstance = malloc(sizeof(SEM_TypeInstance));
	typeInstance->typeEnum = SEM_TYPEINST_STRUCT;
	typeInstance->declaration = NULL;
	typeInstance->name = name;
	typeInstance->variables = Locic_List_Alloc();
	return typeInstance;
}

