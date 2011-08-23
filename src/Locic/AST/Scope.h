#ifndef LOCIC_AST_SCOPE_H
#define LOCIC_AST_SCOPE_H

#include <Locic/AST/List.h>

typedef struct AST_Scope{
	AST_List * statementList;
} AST_Scope;

inline AST_Scope * AST_MakeScope(AST_List * statementList){
	AST_Scope * scope = malloc(sizeof(AST_Scope));
	scope->statementList = statementList;
	return scope;
}

#endif
