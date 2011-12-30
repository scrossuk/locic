#include <stdlib.h>
#include <Locic/List.h>
#include <Locic/SEM/Scope.h>

SEM_Scope * SEM_MakeScope(Locic_List * statementList){
	SEM_Scope * scope = malloc(sizeof(SEM_Scope));
	scope->statementList = statementList;
	return scope;
}

