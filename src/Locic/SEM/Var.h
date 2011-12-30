#ifndef LOCIC_SEM_VAR_H
#define LOCIC_SEM_VAR_H

#include <Locic/SEM/Type.h>

typedef enum SEM_VarType{
	SEM_VAR_LOCAL,
	SEM_VAR_THIS
} SEM_VarType;

typedef struct SEM_Var{
	SEM_Type * type;
	SEM_VarType varType;
	size_t varId;
} SEM_Var;

SEM_Var * SEM_MakeLocalVar(size_t id, SEM_Type * type);

SEM_Var * SEM_MakeThisVar(size_t id, SEM_Type * type);

#endif
