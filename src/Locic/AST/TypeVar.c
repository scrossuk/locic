#include <stdlib.h>
#include <Locic/AST/Type.h>
#include <Locic/AST/TypeVar.h>

AST_TypeVar * AST_MakeTypeVar(AST_Type * type, char * name){
	AST_TypeVar * typeVar = malloc(sizeof(AST_TypeVar));
	typeVar->type = type;
	typeVar->name = name;
	return typeVar;
}

