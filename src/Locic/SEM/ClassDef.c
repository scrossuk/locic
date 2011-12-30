#include <stdio.h>
#include <stdlib.h>
#include <Locic/SEM/ClassDef.h>
#include <Locic/List.h>

SEM_ClassDef * SEM_MakeClassDef(char * name, Locic_List * members, Locic_List * definitions){
	SEM_ClassDef * classDef = malloc(sizeof(SEM_ClassDef));
	classDef->name = name;
	classDef->memberVariables = members;
	classDef->methodDefinitions = definitions;
	return classDef;
}

void SEM_PrintClassDef(SEM_ClassDef * def){
	printf("class %s(...){\n...\n}\n", def->name);
}

