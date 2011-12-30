#include <stdio.h>
#include <stdlib.h>
#include <Locic/SEM/ClassDef.h>
#include <Locic/List.h>

SEM_ClassDef * SEM_MakeClassDef(SEM_ClassDecl * declaration, Locic_List * memberVariables, Locic_List * methodDefinitions){
	SEM_ClassDef * classDef = malloc(sizeof(SEM_ClassDef));
	classDef->declaration = declaration;
	classDef->memberVariables = memberVariables;
	classDef->methodDefinitions = methodDefinitions;
	return classDef;
}

