#include <stdlib.h>
#include <Locic/SEM/Type.h>
#include <Locic/SEM/Var.h>

inline SEM_Var * SEM_AllocateVar(SEM_VarType varType, SEM_Type type){
	SEM_Var * var = malloc(sizeof(SEM_Var));
	var->type = type;
	var->varType = varType;
	return var;
}

SEM_Var * SEM_MakeLocalVar(size_t id, SEM_Type type){
	SEM_Var * var = SEM_AllocateVar(SEM_VAR_LOCAL, type);
	(var->localVar).id = id;
	return var;
}

SEM_Var * SEM_MakeThisVar(size_t id, SEM_Type type){
	SEM_Var * var = SEM_AllocateVar(SEM_VAR_THIS, type);
	(var->thisVar).id = id;
	return var;
}

