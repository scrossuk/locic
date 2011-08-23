#ifndef LOCIC_AST_VAR_H
#define LOCIC_AST_VAR_H

typedef struct AST_LocalVar{
	char * name;
} AST_LocalVar;

typedef struct AST_ThisVar{
	char * name;
} AST_ThisVar;

enum AST_VarType{
	AST_VAR_LOCAL,
	AST_VAR_THIS
};

typedef struct AST_Var{
	AST_VarType type;
	
	union{
		AST_LocalVar localVar;
		AST_ThisVar thisVar;
	};
} AST_Var;

inline AST_Var * AST_AllocateVar(AST_VarType type){
	AST_Var * var = malloc(sizeof(AST_Var));
	var->type = type;
	return var;
}

inline AST_Var * AST_MakeLocalVar(char * name){
	AST_Var * var = AST_AllocateVar(AST_VAR_LOCAL);
	(var->localVar).name = name;
	return var;
}

inline AST_Var * AST_MakeThisVar(char * name){
	AST_Var * var = AST_AllocateVar(AST_VAR_THIS);
	(var->thisVar).name = name;
	return var;
}

#endif
