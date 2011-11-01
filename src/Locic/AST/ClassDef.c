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

