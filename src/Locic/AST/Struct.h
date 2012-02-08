#ifndef LOCIC_AST_STRUCT_H
#define LOCIC_AST_STRUCT_H

#include <Locic/List.h>

typedef struct AST_Struct{
	char * name;
	Locic_List * variables;
} AST_Struct;

AST_Struct * AST_MakeStruct(char * name, Locic_List * variables);

void AST_PrintStruct(AST_Struct * astStruct);

#endif
