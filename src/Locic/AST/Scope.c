#include <stdlib.h>
#include <Locic/List.h>
#include <Locic/AST/Scope.h>

AST_Scope * AST_MakeScope(Locic_List * statementList){
	AST_Scope * scope = malloc(sizeof(AST_Scope));
	scope->statementList = statementList;
	return scope;
}

