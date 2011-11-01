#ifndef LOCIC_AST_STATEMENT_H
#define LOCIC_AST_STATEMENT_H

#include <Locic/AST/Scope.h>
#include <Locic/AST/Type.h>
#include <Locic/AST/Value.h>
#include <Locic/AST/Var.h>

typedef struct AST_ValueStmt{
	AST_Value * value;
} AST_ValueStmt;
		
typedef struct AST_IfStmt{
	AST_Value * cond;
	AST_Scope * ifTrue;
	AST_Scope * ifFalse;
} AST_IfStmt;
		
typedef struct AST_VarDecl{
	AST_Type * type; // NULL when the keyword 'auto' is used.
	char * varName;
	AST_Value * value;
} AST_VarDecl;

typedef struct AST_AssignVar{
	AST_Var * var;
	AST_Value * value;
} AST_AssignVar;

typedef struct AST_ReturnStmt{
	AST_Value * value;
} AST_ReturnStmt;

typedef enum AST_StatementType{
	AST_STATEMENT_VALUE,
	AST_STATEMENT_IF,
	AST_STATEMENT_VARDECL,
	AST_STATEMENT_ASSIGNVAR,
	AST_STATEMENT_RETURN
} AST_StatementType;
	
typedef struct AST_Statement{
	AST_StatementType type;
			
	union{
		AST_ValueStmt valueStmt;
		AST_IfStmt ifStmt;
		AST_VarDecl varDecl;
		AST_AssignVar assignVar;
		AST_ReturnStmt returnStmt;
	};
} AST_Statement;

AST_Statement * AST_MakeValueStmt(AST_Value * value);

AST_Statement * AST_MakeIf(AST_Value * cond, AST_Scope * ifTrue, AST_Scope * ifFalse);

AST_Statement * AST_MakeVarDecl(AST_Type * type, char * varName, AST_Value * value);

AST_Statement * AST_MakeAutoVarDecl(char * varName, AST_Value * value);

AST_Statement * AST_MakeAssignVar(AST_Var * var, AST_Value * value);

AST_Statement * AST_MakeReturn(AST_Value * value);

#endif
