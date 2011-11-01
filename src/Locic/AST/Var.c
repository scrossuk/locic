#include <stdlib.h>
#include <Locic/AST/Var.h>

inline AST_Var * AST_AllocateVar(AST_VarType type){
	AST_Var * var = malloc(sizeof(AST_Var));
	var->type = type;
	return var;
}

AST_Var * AST_MakeLocalVar(char * name){
	AST_Var * var = AST_AllocateVar(AST_VAR_LOCAL);
	(var->localVar).name = name;
	return var;
}

AST_Var * AST_MakeThisVar(char * name){
	AST_Var * var = AST_AllocateVar(AST_VAR_THIS);
	(var->thisVar).name = name;
	return var;
}

