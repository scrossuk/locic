#include <stdlib.h>
#include <Locic/SEM/Var.h>

inline SEM_Var * SEM_AllocateVar(SEM_VarType type){
	SEM_Var * var = malloc(sizeof(SEM_Var));
	var->type = type;
	return var;
}

SEM_Var * SEM_MakeLocalVar(char * name){
	SEM_Var * var = SEM_AllocateVar(SEM_VAR_LOCAL);
	(var->localVar).name = name;
	return var;
}

SEM_Var * SEM_MakeThisVar(char * name){
	SEM_Var * var = SEM_AllocateVar(SEM_VAR_THIS);
	(var->thisVar).name = name;
	return var;
}

