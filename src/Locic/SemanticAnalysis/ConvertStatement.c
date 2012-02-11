#include <assert.h>
#include <stdio.h>
#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/CanCast.h>
#include <Locic/SemanticAnalysis/Context.h>
#include <Locic/SemanticAnalysis/ConvertScope.h>
#include <Locic/SemanticAnalysis/ConvertType.h>
#include <Locic/SemanticAnalysis/ConvertValue.h>

int Locic_SemanticAnalysis_WillStatementReturn(SEM_Statement * statement){
	switch(statement->type){
		case SEM_STATEMENT_VALUE:
		{
			return 0;
		}
		case SEM_STATEMENT_SCOPE:
		{
			return Locic_SemanticAnalysis_WillScopeReturn(statement->scopeStmt.scope);
		}
		case SEM_STATEMENT_IF:
		{
			if(Locic_SemanticAnalysis_WillScopeReturn(statement->ifStmt.ifTrue) == 1 &&
				Locic_SemanticAnalysis_WillScopeReturn(statement->ifStmt.ifFalse) == 1){
				return 1;
			}else{
				return 0;
			}
		}
		case SEM_STATEMENT_WHILE:
		{
			return Locic_SemanticAnalysis_WillScopeReturn(statement->whileStmt.whileTrue);
		}
		case SEM_STATEMENT_ASSIGN:
		{
			return 0;
		}
		case SEM_STATEMENT_RETURN:
		{
			return 1;
		}
		default:
		{
			printf("Internal Compiler Error: Unknown statement type in Locic_SemanticAnalysis_WillStatementReturn.\n");
			return 0;
		}
	}
}

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
		case AST_STATEMENT_SCOPE:
		{
			SEM_Scope * scope = Locic_SemanticAnalysis_ConvertScope(context, statement->scopeStmt.scope);
			if(scope == NULL) return NULL;
			
			return SEM_MakeScopeStmt(scope);
		}
		case AST_STATEMENT_IF:
		{
			SEM_Value * cond = Locic_SemanticAnalysis_ConvertValue(context, statement->ifStmt.cond);
			SEM_Scope * ifTrue = Locic_SemanticAnalysis_ConvertScope(context, statement->ifStmt.ifTrue);
			SEM_Scope * ifFalse = (statement->ifStmt.ifFalse != NULL ? Locic_SemanticAnalysis_ConvertScope(context, statement->ifStmt.ifFalse) : SEM_MakeScope());
			
			if(cond == NULL || ifTrue == NULL || ifFalse == NULL) return NULL;
			
			SEM_Type * boolType = SEM_MakeBasicType(SEM_TYPE_CONST, SEM_TYPE_RVALUE, SEM_TYPE_BASIC_BOOL);
			
			SEM_Value * boolValue = Locic_SemanticAnalysis_CastValueToType(context, cond, boolType);
			
			if(boolValue == NULL){
				printf("Semantic Analysis Error: Cannot cast or copy condition type (");
				SEM_PrintType(cond->type);
				printf(") to bool type in IF statement.\n");
				return NULL;
			}
			
			return SEM_MakeIf(boolValue, ifTrue, ifFalse);
		}
		case AST_STATEMENT_WHILE:
		{
			SEM_Value * cond = Locic_SemanticAnalysis_ConvertValue(context, statement->whileStmt.cond);
			SEM_Scope * whileTrue = Locic_SemanticAnalysis_ConvertScope(context, statement->whileStmt.whileTrue);
			
			if(cond == NULL || whileTrue == NULL) return NULL;
			
			SEM_Type * boolType = SEM_MakeBasicType(SEM_TYPE_CONST, SEM_TYPE_RVALUE, SEM_TYPE_BASIC_BOOL);
			
			SEM_Value * boolValue = Locic_SemanticAnalysis_CastValueToType(context, cond, boolType);
			
			if(boolValue == NULL){
				printf("Semantic Analysis Error: Cannot cast or copy condition type (");
				SEM_PrintType(cond->type);
				printf(") to bool type in WHILE statement.\n");
				return NULL;
			}
			
			return SEM_MakeWhile(boolValue, whileTrue);
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
			
			int canCopyValue = Locic_SemanticAnalysis_CanDoImplicitCopy(context, semValue->type);
			
			SEM_Type * type;
			
			if(typeAnnotation == NULL){
				// Auto keyword - use type of initial value.
				type = SEM_CopyType(semValue->type);
				type->isMutable = SEM_TYPE_MUTABLE;
				type->isLValue = SEM_TYPE_LVALUE;
				if(semValue->type->isMutable == SEM_TYPE_CONST){
					if(canCopyValue == 0){
						type->isMutable = SEM_TYPE_CONST;
					}
				}
			}else{
				// Using type annotation - verify that it is compatible with the type of the initial value.
				type = Locic_SemanticAnalysis_ConvertType(context, typeAnnotation, SEM_TYPE_LVALUE);
				if(type == NULL){
					return NULL;
				}
			}
			
			if(type->typeEnum == SEM_TYPE_VOID){
				printf("Semantic Analysis Error: Local variable cannot have void type.\n");
				return NULL;
			}
			
			SEM_Var * semVar = Locic_SemanticContext_DefineLocalVar(context, varName, type);
			if(semVar == NULL){
				printf("Semantic Analysis Error: Local variable name already exists.\n");
				return NULL;
			}
			
			SEM_Value * castValue = Locic_SemanticAnalysis_CastValueToType(context, semValue, type);
			
			if(castValue == NULL){
				assert(typeAnnotation != NULL);
				printf("Semantic Analysis Error: Cannot cast or copy value type (");
				SEM_PrintType(semValue->type);
				printf(") to annotated variable type (");
				SEM_PrintType(type);
				printf(") in declaration.\n");
				return NULL;
			}
			
			return SEM_MakeAssign(SEM_MakeVarValue(semVar), castValue);
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
			
			SEM_Value * castRValue = Locic_SemanticAnalysis_CastValueToType(context, rValue, lValue->type);
			if(castRValue == NULL){
				printf("Semantic Analysis Error: Cannot cast or copy rvalue type (");
				SEM_PrintType(rValue->type);
				printf(") to lvalue type (");
				SEM_PrintType(lValue->type);
				printf(") in assignment.\n");
				return NULL;
			}
			
			return SEM_MakeAssign(lValue, castRValue);
		}
		case AST_STATEMENT_RETURN:
		{
			if(statement->returnStmt.value == NULL){
				// Void return statement.
				if(SEM_IsVoidType(context->functionDecl->type->funcType.returnType) == 0){
					printf("Semantic Analysis Error: Cannot return void in function with non-void return type.\n");
					return NULL;
				}
				return SEM_MakeReturn(NULL);
			}else{
				SEM_Value * semValue = Locic_SemanticAnalysis_ConvertValue(context, statement->returnStmt.value);
				if(semValue == NULL){
					return NULL;
				}
				
				SEM_Value * castValue = Locic_SemanticAnalysis_CastValueToType(context, semValue, context->functionDecl->type->funcType.returnType);
				
				if(castValue == NULL){
					printf("Semantic Analysis Error: Cannot cast or copy value type (");
					SEM_PrintType(semValue->type);
					printf(") to function return type (");
					SEM_PrintType(context->functionDecl->type->funcType.returnType);
					printf(") in return statement.\n");
					return NULL;
				}
				
				return SEM_MakeReturn(castValue);
			}
		}
		default:
			printf("Internal Compiler Error: Unknown statement type in 'Locic_SemanticAnalysis_ConvertStatement'.\n");
			return NULL;
	}
}


