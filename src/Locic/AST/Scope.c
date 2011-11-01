#include <stdlib.h>
#include <Locic/AST/List.h>
#include <Locic/AST/Scope.h>

AST_Scope * AST_MakeScope(AST_List * statementList){
	AST_Scope * scope = malloc(sizeof(AST_Scope));
	scope->statementList = statementList;
	return scope;
}

