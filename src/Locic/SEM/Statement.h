#ifndef LOCIC_SEM_STATEMENT_H
#define LOCIC_SEM_STATEMENT_H

#include <Locic/SEM/Scope.h>
#include <Locic/SEM/Type.h>
#include <Locic/SEM/Value.h>
#include <Locic/SEM/Var.h>

typedef struct SEM_ValueStmt{
	SEM_Value * value;
} SEM_ValueStmt;
		
typedef struct SEM_IfStmt{
	SEM_Value * cond;
	SEM_Scope * ifTrue;
	SEM_Scope * ifFalse;
} SEM_IfStmt;

typedef struct SEM_AssignStmt{
	SEM_Value * lValue, * rValue;
} SEM_AssignStmt;

typedef struct SEM_ReturnStmt{
	SEM_Value * value;
} SEM_ReturnStmt;

typedef enum SEM_StatementType{
	SEM_STATEMENT_VALUE,
	SEM_STATEMENT_IF,
	SEM_STATEMENT_ASSIGN,
	SEM_STATEMENT_RETURN
} SEM_StatementType;
	
typedef struct SEM_Statement{
	SEM_StatementType type;
			
	union{
		SEM_ValueStmt valueStmt;
		SEM_IfStmt ifStmt;
		SEM_AssignStmt assignStmt;
		SEM_ReturnStmt returnStmt;
	};
} SEM_Statement;

SEM_Statement * SEM_MakeValueStmt(SEM_Value * value);

SEM_Statement * SEM_MakeIf(SEM_Value * cond, SEM_Scope * ifTrue, SEM_Scope * ifFalse);

SEM_Statement * SEM_MakeAssign(SEM_Value * lValue, SEM_Value * rValue);

SEM_Statement * SEM_MakeReturn(SEM_Value * value);

#endif
