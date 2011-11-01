#ifndef LOCIC_AST_TYPEVAR_H
#define LOCIC_AST_TYPEVAR_H

#include <Locic/AST/Type.h>

typedef struct AST_TypeVar{
	AST_Type * type;
	char * name;
} AST_TypeVar;

AST_TypeVar * AST_MakeTypeVar(AST_Type * type, char * name);

#endif
