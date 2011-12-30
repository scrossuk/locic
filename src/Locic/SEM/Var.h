#ifndef LOCIC_SEM_VAR_H
#define LOCIC_SEM_VAR_H

typedef enum SEM_VarType{
	SEM_VAR_LOCAL,
	SEM_VAR_THIS
} SEM_VarType;

typedef struct SEM_Var{
	SEM_VarType type;
	size_t varId;
} SEM_Var;

SEM_Var * SEM_MakeLocalVar(char * name);

SEM_Var * SEM_MakeThisVar(char * name);

#endif
