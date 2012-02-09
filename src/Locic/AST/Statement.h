#ifndef LOCIC_AST_STATEMENT_H
#define LOCIC_AST_STATEMENT_H

#include <Locic/AST/Scope.h>
#include <Locic/AST/Type.h>
#include <Locic/AST/Value.h>
#include <Locic/AST/Var.h>

typedef struct AST_ValueStmt{
	AST_Value * value;
} AST_ValueStmt;

typedef struct AST_ScopeStmt{
	AST_Scope * scope;
} AST_ScopeStmt;
		
typedef struct AST_IfStmt{
	AST_Value * cond;
	AST_Scope * ifTrue;
	AST_Scope * ifFalse;
} AST_IfStmt;

typedef struct AST_WhileStmt{
	AST_Value * cond;
	AST_Scope * whileTrue;
} AST_WhileStmt;
		
typedef struct AST_VarDecl{
	AST_Type * type; // NULL when the keyword 'auto' is used.
	char * varName;
	AST_Value * value;
} AST_VarDecl;

typedef struct AST_AssignStmt{
	AST_Value * lValue, * rValue;
} AST_AssignStmt;

typedef struct AST_ReturnStmt{
	AST_Value * value;
} AST_ReturnStmt;

typedef enum AST_StatementType{
	AST_STATEMENT_VALUE,
	AST_STATEMENT_SCOPE,
	AST_STATEMENT_IF,
	AST_STATEMENT_WHILE,
	AST_STATEMENT_VARDECL,
	AST_STATEMENT_ASSIGN,
	AST_STATEMENT_RETURN
} AST_StatementType;
	
typedef struct AST_Statement{
	AST_StatementType type;
			
	union{
		AST_ValueStmt valueStmt;
		AST_ScopeStmt scopeStmt;
		AST_IfStmt ifStmt;
		AST_WhileStmt whileStmt;
		AST_VarDecl varDecl;
		AST_AssignStmt assignStmt;
		AST_ReturnStmt returnStmt;
	};
} AST_Statement;

AST_Statement * AST_MakeValueStmt(AST_Value * value);

AST_Statement * AST_MakeScopeStmt(AST_Scope * scope);

AST_Statement * AST_MakeIf(AST_Value * cond, AST_Scope * ifTrue, AST_Scope * ifFalse);

AST_Statement * AST_MakeWhile(AST_Value * cond, AST_Scope * whileTrue);

AST_Statement * AST_MakeVarDecl(AST_Type * type, char * varName, AST_Value * value);

AST_Statement * AST_MakeAutoVarDecl(char * varName, AST_Value * value);

AST_Statement * AST_MakeAssign(AST_Value * lValue, AST_Value * rValue);

AST_Statement * AST_MakeReturn(AST_Value * value);

#endif
