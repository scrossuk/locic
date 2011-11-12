#include <stdio.h>
#include <stdlib.h>
#include <Locic/AST/ClassDef.h>
#include <Locic/AST/List.h>

AST_ClassDef * AST_MakeClassDef(char * name, AST_List * members, AST_List * definitions){
	AST_ClassDef * classDef = malloc(sizeof(AST_ClassDef));
	classDef->name = name;
	classDef->memberVariables = members;
	classDef->methodDefinitions = definitions;
	return classDef;
}

void AST_PrintClassDef(AST_ClassDef * def){
	printf("class %s(...){\n...\n}\n", def->name);
}

