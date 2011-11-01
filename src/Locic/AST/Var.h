#ifndef LOCIC_AST_VAR_H
#define LOCIC_AST_VAR_H

typedef struct AST_LocalVar{
	char * name;
} AST_LocalVar;

typedef struct AST_ThisVar{
	char * name;
} AST_ThisVar;

typedef enum AST_VarType{
	AST_VAR_LOCAL,
	AST_VAR_THIS
} AST_VarType;

typedef struct AST_Var{
	AST_VarType type;
	
	union{
		AST_LocalVar localVar;
		AST_ThisVar thisVar;
	};
} AST_Var;

AST_Var * AST_MakeLocalVar(char * name);

AST_Var * AST_MakeThisVar(char * name);

#endif
