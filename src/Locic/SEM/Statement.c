#include <stdlib.h>
#include <Locic/SEM/Scope.h>
#include <Locic/SEM/Statement.h>
#include <Locic/SEM/Value.h>
#include <Locic/SEM/Var.h>

inline SEM_Statement * SEM_AllocateStatement(SEM_StatementType type){
	SEM_Statement * statement = malloc(sizeof(SEM_Statement));
	statement->type = type;
	return statement;
}

/* SEM_ValueStmt */

SEM_Statement * SEM_MakeValueStmt(SEM_Value * value){
	SEM_Statement * statement = SEM_AllocateStatement(SEM_STATEMENT_VALUE);
	SEM_ValueStmt * valueStmt = &(statement->valueStmt);
	valueStmt->value = value;
	return statement;
}

/* SEM_IfStmt */

SEM_Statement * SEM_MakeIf(SEM_Value * cond, SEM_Scope * ifTrue, SEM_Scope * ifFalse){
	SEM_Statement * statement = SEM_AllocateStatement(SEM_STATEMENT_IF);
	SEM_IfStmt * ifStmt = &(statement->ifStmt);
	ifStmt->cond = cond;
	ifStmt->ifTrue = ifTrue;
	ifStmt->ifFalse = ifFalse;
	return statement;
}

/* SEM_AssignVar */

SEM_Statement * SEM_MakeAssignVar(SEM_Var * var, SEM_Value * value){
	SEM_Statement * statement = SEM_AllocateStatement(SEM_STATEMENT_ASSIGNVAR);
	SEM_AssignVar * assignVar = &(statement->assignVar);
	assignVar->var = var;
	assignVar->value = value;
	return statement;
}

/* SEM_ReturnStmt */

SEM_Statement * SEM_MakeReturn(SEM_Value * value){
	SEM_Statement * statement = SEM_AllocateStatement(SEM_STATEMENT_RETURN);
	SEM_ReturnStmt * returnStmt = &(statement->returnStmt);
	returnStmt->value = value;
	return statement;
}

