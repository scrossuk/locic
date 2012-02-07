#include <stdio.h>
#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/CanCast.h>
#include <Locic/SemanticAnalysis/Context.h>
#include <Locic/SemanticAnalysis/ConvertScope.h>
#include <Locic/SemanticAnalysis/ConvertType.h>
#include <Locic/SemanticAnalysis/ConvertValue.h>

SEM_Statement * Locic_SemanticAnalysis_ConvertStatement(Locic_SemanticContext * context, AST_Statement * statement){
	switch(statement->type){
		case AST_STATEMENT_VALUE:
		{
			SEM_Value * value = Locic_SemanticAnalysis_ConvertValue(context, statement->valueStmt.value);
			if(value != NULL){
				return SEM_MakeValueStmt(value);
			}
			return NULL;
		}
		case AST_STATEMENT_IF:
		{
			SEM_Value * cond = Locic_SemanticAnalysis_ConvertValue(context, statement->ifStmt.cond);
			SEM_Scope * ifTrue = Locic_SemanticAnalysis_ConvertScope(context, statement->ifStmt.ifTrue);
			SEM_Scope * ifFalse = (statement->ifStmt.ifFalse != NULL ? Locic_SemanticAnalysis_ConvertScope(context, statement->ifStmt.ifFalse) : NULL);
			
			if(cond == NULL || ifTrue == NULL || (statement->ifStmt.ifFalse != NULL && ifFalse == NULL)) return NULL;
			
			SEM_Type * boolType = SEM_MakeBasicType(SEM_TYPE_CONST, SEM_TYPE_RVALUE, SEM_TYPE_BASIC_BOOL);
			
			if(Locic_SemanticAnalysis_CanDoImplicitCast(context, cond->type, boolType) == 1){
				return SEM_MakeIf(cond, ifTrue, ifFalse);
			}else{
				printf("Semantic Analysis Error: Cannot convert condition expression to boolean type in IF statement.\n");
				return NULL;
			}
		}
		case AST_STATEMENT_VARDECL:
		{
			AST_Type * typeAnnotation = statement->varDecl.type;
			char * varName = statement->varDecl.varName;
			AST_Value * initialValue = statement->varDecl.value;
			
			SEM_Value * semValue = Locic_SemanticAnalysis_ConvertValue(context, initialValue);
			if(semValue == NULL){
				return NULL;
			}
			
			if(semValue->type->isLValue == SEM_TYPE_LVALUE){
				if(Locic_SemanticAnalysis_CanDoImplicitCopy(context, semValue->type)){
					// If possible, an implicit copy can create an r-value.
					semValue = SEM_MakeCopyValue(semValue);
				}else{
					printf("Semantic Analysis Error: Cannot assign l-value in declaration (must be copied or emptied).\n");
					return NULL;
				}
			}
			
			SEM_Type * type;
			
			if(typeAnnotation == NULL){
				// Auto keyword - use type of initial value.
				type = SEM_CopyType(semValue->type);
				
				type->isLValue = SEM_TYPE_LVALUE;
			}else{
				// Using type annotation - verify that it is compatible with the type of the initial value.
				type = Locic_SemanticAnalysis_ConvertType(context, typeAnnotation, SEM_TYPE_LVALUE);
				if(type == NULL){
					return NULL;
				}
				
				if(!Locic_SemanticAnalysis_CanDoImplicitCast(context, semValue->type, type)){
					printf("Semantic Analysis Error: Cannot cast variable's initial value type to annotated type in declaration.\n");
					return NULL;
				}
			}
			
			SEM_Var * semVar = Locic_SemanticContext_DefineLocalVar(context, varName, type);
			if(semVar == NULL){
				printf("Semantic Analysis Error: Local variable name already exists.\n");
				return NULL;
			}
			
			return SEM_MakeAssign(SEM_MakeVarValue(semVar), semValue);
		}
		case AST_STATEMENT_ASSIGN:
		{
			SEM_Value * lValue = Locic_SemanticAnalysis_ConvertValue(context, statement->assignStmt.lValue);
			if(lValue == NULL){
				return NULL;
			}
			
			if(lValue->type->isMutable == SEM_TYPE_CONST){
				printf("Semantic Analysis Error: Cannot assign to const value.\n");
				return NULL;
			}
			
			if(lValue->type->isLValue == SEM_TYPE_RVALUE){
				printf("Semantic Analysis Error: Cannot assign to r-value.\n");
				return NULL;
			}
			
			SEM_Value * rValue = Locic_SemanticAnalysis_ConvertValue(context, statement->assignStmt.rValue);
			if(rValue == NULL){
				return NULL;
			}
			
			if(rValue->type->isLValue == SEM_TYPE_LVALUE){
				if(Locic_SemanticAnalysis_CanDoImplicitCopy(context, rValue->type)){
					// If possible, an implicit copy can create an r-value.
					rValue = SEM_MakeCopyValue(rValue);
				}else{
					printf("Semantic Analysis Error: Cannot assign l-value (must be copied or emptied).\n");
					return NULL;
				}
			}
			
			if(!Locic_SemanticAnalysis_CanDoImplicitCast(context, rValue->type, lValue->type)){
				printf("Semantic Analysis Error: Cannot cast r-value to l-value's type in assignment statement.\n");
				return NULL;
			}
			
			return SEM_MakeAssign(lValue, rValue);
		}
		case AST_STATEMENT_RETURN:
		{
			if(statement->returnStmt.value == NULL){
				printf("Internal compiler error: Cannot return NULL AST_Value.\n");
				return NULL;
			}
			
			SEM_Value * semValue = Locic_SemanticAnalysis_ConvertValue(context, statement->returnStmt.value);
			if(semValue == NULL){
				return NULL;
			}
			
			if(!Locic_SemanticAnalysis_CanDoImplicitCast(context, semValue->type, context->functionDecl->type->funcType.returnType)){
				printf("Semantic Analysis Error: Cannot cast value in return statement to function's return type.\n");
				return NULL;
			}
			
			return SEM_MakeReturn(semValue);
		}
		default:
			return NULL;
	}
}


