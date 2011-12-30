#ifndef LOCIC_AST_SCOPE_H
#define LOCIC_AST_SCOPE_H

#include <Locic/List.h>

typedef struct AST_Scope{
	Locic_List * statementList;
} AST_Scope;

AST_Scope * AST_MakeScope(Locic_List * statementList);

#endif
