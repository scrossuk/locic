#ifndef LOCIC_SEM_TYPEINSTANCE_H
#define LOCIC_SEM_TYPEINSTANCE_H

#include <Locic/List.h>

typedef enum SEM_TypeInstanceEnum{
	SEM_TYPEINST_CLASSDECL,
	SEM_TYPEINST_CLASSDEF,
	SEM_TYPEINST_STRUCT
} SEM_TypeInstanceEnum;

typedef struct SEM_TypeInstance{
	SEM_TypeInstanceEnum typeEnum;
	struct SEM_TypeInstance * declaration;
	char * name;
	Locic_List * variables;
} SEM_TypeInstance;

SEM_TypeInstance * SEM_MakeClassDecl(char * name);

SEM_TypeInstance * SEM_MakeClassDef(SEM_TypeInstance * declaration);

SEM_TypeInstance * SEM_MakeStruct(char * name);

#endif
