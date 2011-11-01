#include <stdlib.h>
#include <Locic/AST/Scope.h>
#include <Locic/AST/Statement.h>
#include <Locic/AST/Value.h>
#include <Locic/AST/Var.h>

inline AST_Statement * AST_AllocateStatement(AST_StatementType type){
	AST_Statement * statement = malloc(sizeof(AST_Statement));
	statement->type = type;
	return statement;
}

/* AST_ValueStmt */

AST_Statement * AST_MakeValueStmt(AST_Value * value){
	AST_Statement * statement = AST_AllocateStatement(AST_STATEMENT_VALUE);
	AST_ValueStmt * valueStmt = &(statement->valueStmt);
	valueStmt->value = value;
	return statement;
}

/* AST_IfStmt */

AST_Statement * AST_MakeIf(AST_Value * cond, AST_Scope * ifTrue, AST_Scope * ifFalse){
	AST_Statement * statement = AST_AllocateStatement(AST_STATEMENT_IF);
	AST_IfStmt * ifStmt = &(statement->ifStmt);
	ifStmt->cond = cond;
	ifStmt->ifTrue = ifTrue;
	ifStmt->ifFalse = ifFalse;
	return statement;
}

/* AST_VarDecl */

AST_Statement * AST_MakeVarDecl(AST_Type * type, char * varName, AST_Value * value){
	AST_Statement * statement = AST_AllocateStatement(AST_STATEMENT_VARDECL);
	AST_VarDecl * varDecl = &(statement->varDecl);
	varDecl->type = type;
	varDecl->varName = varName;
	varDecl->value = value;
	return statement;
}

AST_Statement * AST_MakeAutoVarDecl(char * varName, AST_Value * value){
	return AST_MakeVarDecl(NULL, varName, value);
}

/* AST_AssignVar */

AST_Statement * AST_MakeAssignVar(AST_Var * var, AST_Value * value){
	AST_Statement * statement = AST_AllocateStatement(AST_STATEMENT_ASSIGNVAR);
	AST_AssignVar * assignVar = &(statement->assignVar);
	assignVar->var = var;
	assignVar->value = value;
	return statement;
}

/* AST_ReturnStmt */

AST_Statement * AST_MakeReturn(AST_Value * value){
	AST_Statement * statement = AST_AllocateStatement(AST_STATEMENT_RETURN);
	AST_ReturnStmt * returnStmt = &(statement->returnStmt);
	returnStmt->value = value;
	return statement;
}

