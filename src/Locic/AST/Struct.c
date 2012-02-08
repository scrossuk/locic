#include <stdio.h>
#include <stdlib.h>
#include <Locic/AST/Struct.h>
#include <Locic/List.h>

AST_Struct * AST_MakeStruct(char * name, Locic_List * variables){
	AST_Struct * astStruct = malloc(sizeof(AST_Struct));
	astStruct->name = name;
	astStruct->variables = variables;
	return astStruct;
}

void AST_PrintStruct(AST_Struct * astStruct){
	printf("struct %s{\n...\n}\n", astStruct->name);
}

