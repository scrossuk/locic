#ifndef LOCIC_AST_STATEMENT_H
#define LOCIC_AST_STATEMENT_H

#include <Locic/AST/Scope.hpp>
#include <Locic/AST/Value.hpp>
#include <Locic/AST/Var.hpp>

typedef struct AST_ValueStmt{
	AST_Value * value;
} AST_ValueStmt;
		
typedef struct AST_If{
	AST_Value * cond;
	AST_Scope * ifTrue;
	AST_Scope * ifFalse;
} AST_If;
		
typedef struct AST_VarDecl{
	AST_Type * type; // NULL when the keyword 'auto' is used.
	char * varName;
	AST_Value * value;
} AST_VarDecl;

typedef struct AST_AssignVar{
	AST_Var * var;
	AST_Value * value;
} AST_AssignVar;

typedef struct AST_Return{
	AST_Value * value;
} AST_Return;

enum AST_StatementType{
	AST_STATEMENT_VALUE,
	AST_STATEMENT_IF,
	AST_STATEMENT_VARDECL,
	AST_STATEMENT_ASSIGNVAR,
	AST_STATEMENT_RETURN
};
	
typedef struct AST_Statement{
	AST_StatementType type;
			
	union{
		AST_ValueStmt valueStmt;
		AST_If ifStmt;
		AST_VarDecl varDecl;
		AST_AssignVar assignVar;
		AST_Return returnStmt;
	};
} AST_Statement;

inline AST_Statement * AST_AllocateStatement(AST_StatementType type){
	AST_Statement * statement = malloc(sizeof(AST_Statement));
	statement->type = type;
	return statement;
}

/* AST_ValueStmt */

inline AST_Statement * AST_MakeValueStmt(AST_Value * value){
	AST_Statement * statement = AST_AllocateStatement(AST_STATEMENT_VALUE);
	AST_ValueStmt * valueStmt = &(statement->valueStmt);
	valueStmt->value = value;
	return statement;
}

/* AST_IfStmt */

inline AST_Statement * AST_MakeIf(AST_Value * cond, AST_Scope * ifTrue, AST_Scope * ifFalse){
	AST_Statement * statement = AST_AllocateStatement(AST_STATEMENT_IF);
	AST_IfStmt * ifStmt = &(statement->ifStmt);
	ifStmt->cond = cond;
	ifStmt->ifTrue = ifTrue;
	ifStmt->ifFalse = ifFalse;
	return statement;
}

/* AST_VarDecl */

inline AST_Statement * AST_MakeVarDecl(AST_Type * type, char * varName, AST_Value * value){
	AST_Statement * statement = AST_AllocateStatement(AST_STATEMENT_VARDECL);
	AST_VarDecl * varDecl = &(statement->varDecl);
	varDecl->type = type;
	varDecl->varName = varName;
	varDecl->value = value;
	return statement;
}

inline AST_Statement * AST_MakeAutoVarDecl(char * varName, AST_Value * value){
	return AST_MakeVarDecl(NULL, varName, value);
}

/* AST_AssignVar */

inline AST_Statement * AST_MakeAssignVar(AST_Var * var, AST_Value * value){
	AST_Statement * statement = AST_AllocateStatement(AST_STATEMENT_ASSIGNVAR);
	AST_AssignVar * assignVar = &(statement->assignVar);
	assignVar->var = var;
	assignVar->value = value;
	return statement;
}

/* AST_ReturnStmt */

inline AST_Statement * AST_MakeReturn(AST_Value * value){
	AST_Statement * statement = AST_AllocateStatement(AST_STATEMENT_RETURN);
	AST_ReturnStmt * returnStmt = &(statement->returnStmt);
	returnStmt->value = value;
	return statement;
}

#endif
