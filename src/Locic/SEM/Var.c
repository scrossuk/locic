#include <stdlib.h>
#include <Locic/SEM/Type.h>
#include <Locic/SEM/Var.h>

SEM_Var * SEM_MakeVar(SEM_VarType varType, size_t id, SEM_Type * type){
	SEM_Var * var = malloc(sizeof(SEM_Var));
	var->type = type;
	var->varType = varType;
	var->varId = id;
	return var;
}

