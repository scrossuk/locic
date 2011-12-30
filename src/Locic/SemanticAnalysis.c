#include <stdio.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis.h>
#include <Locic/SemanticContext.h>

/* Initial phase: build the semantic structure with function and class declarations (so they can be referenced by the second phase). */

SEM_Context * Locic_SemanticAnalysis_BuildDeclarations(AST_Context * synContext){
	
}

/* In-depth phase: extend the semantic structure with the definitions of functions and class methods. */

SEM_Type * Locic_SemanticAnalysis_ConvertType(Locic_SemanticContext * context, AST_Type * type){
	switch(type->typeEnum){
		case AST_TYPE_BASIC:
		{
			return SEM_MakeBasicType(type->isMutable, type->basicType.typeEnum);
		}
		case AST_TYPE_NAMED:
		{
			SEM_ClassDecl * classDecl = Locic_StringMap_Find(context->classDeclarations, type->namedType.name);
			if(classDecl == NULL){
				printf("Semantic Analysis Error: unknown class type with name '%s'\n", type->namedType.name);
				return NULL;
			}
			
			return SEM_MakeClassType(type->isMutable, classDecl);
		}
		case AST_TYPE_PTR:
		{
			return SEM_MakePtrType(type->isMutable, Locic_SemanticAnalysis_ConvertType(context, type->ptrType.ptrType));
		}
		default:
			return NULL;
	}
}

int Locic_SemanticAnalysis_CanDoImplicitCast(Locic_SemanticContext * context, SEM_Type * sourceType, SEM_Type * destType){
	if(sourceType->typeEnum != destType->typeEnum){
		return 0;
	}
	
	switch(sourceType->typeEnum){
		case SEM_TYPE_BASIC:
		{
			if(sourceType->basicType.typeEnum != destType->basicType.typeEnum){
				return 0;
			}
			return 1;
		}
		case SEM_TYPE_CLASS:
		{
			if(sourceType->classType.classDecl != destType->classType.classDecl){
				printf("Semantic Analysis Error: cannot convert between incompatible class types\n");
				return 0;
			}
			
			if(sourceType->isMutable != destType->isMutable){
				printf("Semantic Analysis Error: cannot convert between l-values and r-values\n");
				return 0;
			}
			
			return 1;
		}
		case SEM_TYPE_PTR:
		{
			// Check for const-correctness.
			if(sourceType->ptrType.ptrType->isMutable == SEM_TYPE_CONST && destType->ptrType.ptrType->isMutable == SEM_TYPE_MUTABLE){
				printf("Semantic Analysis Error: const-correctness violation\n");
				return 0;
			}
			
			if(!Locic_SemanticAnalysis_CanDoImplicitCast(context, sourceType->ptrType.ptrType, destType->ptrType.ptrType)){
				return 0;
			}
			return 1;
		}
		default:
			return 0;
	}
}

SEM_FunctionDef * Locic_SemanticAnalysis_ConvertFunctionDef(Locic_SemanticContext * context, AST_FunctionDef * functionDef){
	// Find the corresponding semantic function declaration.
	SEM_FunctionDecl * semFunctionDecl = Locic_StringMap_Find(context->functionDeclarations, functionDef->declaration->name);
	if(semFunctionDecl == NULL){
		printf("Internal compiler error: semantic function declaration not found for definition.");
		return NULL;
	}
	
	Locic_SemanticContext_StartFunction(context, NULL, semFunctionDecl);
	
	// Add parameters to the current context.
	Locic_List * synParameters, * semParameters;
	Locic_ListElement * synIterator, * semIterator;
	
	// AST information gives parameter names; SEM information gives parameter variable information.
	synParameters = functionDef->declaration->parameters;
	semParameters = semfunctionDecl->parameterVars;
	
	for(synIterator = Locic_List_Begin(synParameters), semIterator = Locic_List_Begin(semParameters);
		synIterator != Locic_List_End(synParameters);
		synIterator = synIterator->next, semIterator = semIterator->next){
		
		AST_TypeVar * typeVar = synIterator->data;
		SEM_Var * paramVar = semIterator->data;
		
		// Create a mapping from the parameter's name to its variable information.
		if(Locic_StringMap_Insert(context->functionContext.parameters, typeVar->name, paramVar) != NULL){
			printf("Semantic Analysis Error: Function parameters share variable names.");
			return NULL;
		}
	}
	
	// Generate the outer function scope.
	SEM_Scope * scope = Locic_SemanticAnalysis_ConvertScope(context, functionDef->scope);
	
	if(scope == NULL){
		return NULL;
	}
	
	// Build and return the function definition.
	return SEM_MakeFunctionDef(semFunctionDecl, scope)
}

SEM_Scope * Locic_SemanticAnalysis_ConvertScope(Locic_SemanticContext * context, AST_Scope * scope){
	SEM_Scope * semScope = SEM_MakeScope();

	// Add this scope to the context, so that variables can be added to it.
	Locic_SemanticContext_PushScope(context, semScope);
	
	// Go through each syntactic statement, and create a corresponding semantic statement.
	Locic_List * synStatements = scope->statementList;
	for(Locic_ListElement * it = Locic_List_Begin(synStatement); it != Locic_List_End(synStatement); it = it->next){
		SEM_Statement * statement = Locic_SemanticAnalysis_ConvertStatement(context, it->data);
		if(statement == NULL){
			return NULL;
		}
		
		// Add the new statement to the scope.
		Locic_List_Append(semScope->statementList, statement);
	}
	
	// Remove this scope from the context.
	Locic_SemanticContext_PopScope(context);
	
	return semScope;
}

SEM_Var * Locic_SemanticAnalysis_ConvertVar(Locic_SemanticContext * context, AST_Var * var){
	switch(var->type){
		case AST_VAR_LOCAL:
		{
			SEM_Var * semVar = Locic_SemanticContext_FindLocalVar(var->localVar.name);
			if(semVar != NULL){
				return semVar;
			}
			
			printf("Semantic Analysis Error: Local variable '%s' was not found\n", var->localVar.name);
			return NULL; 
		}
		case AST_VAR_THIS:
		{
			printf("Semantic Analysis Error: Member variables not implemented\n");
			return NULL;
		}
		default:
			return NULL;
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
		case AST_STATEMENT_IF:
		{
			return NULL;
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
			
			if(semValue->type->typeEnum == SEM_TYPE_CLASS){
				if(semValue->type->isMutable == SEM_TYPE_MUTABLE){
					printf("Semantic Analysis Error: Cannot assign l-value to variable.\n");
					return NULL;
				}
			}
			
			SEM_Type * type;
			
			if(typeAnnotation == NULL){
				// Auto keyword - use type of initial value.
				type = semValue->type;
				
				if(type->typeEnum == SEM_TYPE_CLASS){
					type->isMutable = SEM_TYPE_MUTABLE;
				}
			}else{
				// Using type annotation - verify that it is compatible with the type of the initial value.
				type = Locic_SemanticAnalysis_ConvertType(context, typeAnnotation);
				if(type == NULL){
					return NULL;
				}
				
				if(!Locic_SemanticAnalysis_CanDoImplicitCast(context, semValue->type, type)){
					printf("Semantic Analysis Error: Cannot cast variable's initial value type to annotated type.\n");
					return NULL;
				}
			}
			
			SEM_Var * semVar = Locic_SemanticContext_DefineLocalVar(context, varName, type);
			if(semVar == NULL){
				printf("Semantic Analysis Error: Local variable name already exists\n");
				return NULL;
			}
			return SEM_MakeAssignVar(semVar, semValue);
		}
		case AST_STATEMENT_ASSIGNVAR:
		{
			SEM_Var * semVar = Locic_SemanticAnalysis_ConvertVar(statement->assignVar.var);
			if(semVar == NULL){
				return NULL;
			}
			
			SEM_Value * semValue = Locic_SemanticAnalysis_ConvertValue(context, statement->assignVar.value);
			if(semValue == NULL){
				return NULL;
			}
			
			if(!Locic_SemanticAnalysis_CanDoImplicitCast(context, semValue->type, semVar->type)){
				printf("Semantic Analysis Error: Cannot cast value to variable's type in assignment statement.\n");
				return NULL;
			}
			
			return SEM_MakeAssignVar(semVar, semValue);
		}
		case AST_STATEMENT_RETURN:
		{
			SEM_Value * semValue = Locic_SemanticAnalysis_ConvertValue(context, statement->assignVar.value);
			if(semValue == NULL){
				return NULL;
			}
			
			if(!Locic_SemanticAnalysis_CanDoImplicitCast(context, semValue->type, context->functionDecl->returnType)){
				printf("Semantic Analysis Error: Cannot cast value in return statement to function's return type.\n");
				return NULL;
			}
			
			return SEM_MakeReturn(semValue);
		}
		default:
			return NULL;
	}
}

SEM_Value * Locic_SemanticAnalysis_ConvertValue(Locic_SemanticContext * context, AST_Value * value){
	switch(value->type){
		case AST_VALUE_CONSTANT:
		{
			switch(value->constant.type){
				case AST_CONSTANT_BOOL:
					return SEM_MakeBoolConstant(value->constant.boolConstant);
				case AST_CONSTANT_INT:
					return SEM_MakeIntConstant(value->constant.intConstant);
				case AST_CONSTANT_FLOAT:
					return SEM_MakeFloatConstant(value->constant.floatConstant);
				default:
					return NULL;
			}
		}
		case AST_VALUE_VARACCESS:
		{
			SEM_Var * var = Locic_SEM_ConvertVar(context, value->varAccess.var);
			if(var == NULL){
				return NULL;
			}
			return SEM_MakeVarAccess(var);
		}
		case AST_VALUE_UNARY:
		{
			SEM_Value * operand = Locic_SEM_ConvertValue(context, value->unary.value);
			if(operand == NULL){
				return NULL;
			}
			
			switch(value->unary.type){
				case AST_UNARY_PLUS:
				{
					if(operand->type.typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum basicType = operand->type.basicType.typeEnum;
						if(basicType == SEM_TYPE_BASIC_INT || basicType == SEM_TYPE_BASIC_FLOAT){
							return SEM_MakeUnary(SEM_UNARY_PLUS, operand, operand->type);
						}
					}
					printf("Semantic Analysis Error: Unary plus on non-numeric type\n");
					return NULL;
				}
				case AST_UNARY_MINUS:
				{
					if(operand->type.typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum basicType = operand->type.basicType.typeEnum;
						if(basicType == SEM_TYPE_BASIC_INT || basicType == SEM_TYPE_BASIC_FLOAT){
							return SEM_MakeUnary(SEM_UNARY_MINUS, operand, operand->type);
						}
					}
					printf("Semantic Analysis Error: Unary minus on non-numeric type\n");
					return NULL;
				}
				case AST_UNARY_ADDRESSOF:
				{
					return SEM_MakeUnary(SEM_UNARY_ADDRESSOF, operand, SEM_MakePtrType(SEM_TYPE_MUTABLE, operand->type));
				}
				case AST_UNARY_DEREF:
				{
					if(operand->type.typeEnum != SEM_TYPE_PTR){
						return SEM_MakeUnary(SEM_UNARY_DEREF, operand, SEM_MakePtrType(SEM_TYPE_MUTABLE, operand->type));
					}
					
					printf("Semantic Analysis Error: Attempting to dereference non-pointer type\n");
					return NULL;
				}
				case AST_UNARY_NEGATE:
				{
					if(operand->type.typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum basicType = operand->type.basicType.typeEnum;
						if(basicType == SEM_TYPE_BASIC_INT || basicType == SEM_TYPE_BASIC_FLOAT){
							return SEM_MakeUnary(SEM_UNARY_NEGATE, operand, operand->type);
						}
					}
					printf("Semantic Analysis Error: Negation on non-numeric type\n");
					return NULL;
				}
				default:
					return NULL;
			}
		}
		case AST_VALUE_BINARY:
		{
			SEM_Value * leftOperand, * rightOperand;
			leftOperand = Locic_SEM_ConvertValue(context, value->binary.left);
			rightOperand = Locic_SEM_ConvertValue(context, value->binary.right);
			if(leftOperand == NULL || rightOperand == NULL){
				return NULL;
			}
			
			switch(value->binary.type){
				case AST_BINARY_ADD:
				{
					if(leftOperand->type.typeEnum == SEM_TYPE_BASIC && rightOperand->type.typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum leftBasicType, rightBasicType;
						leftBasicType = leftOperand->type.basicType.typeEnum;
						rightBasicType = rightOperand->type.basicType.typeEnum;
						if((leftBasicType == SEM_TYPE_BASIC_INT || leftBasicType == SEM_TYPE_BASIC_FLOAT)
							&& leftBasicType == rightBasicType){
							return SEM_MakeBinary(SEM_BINARY_ADD, leftOperand, rightOperand, leftOperand->type);
						}
					}
					printf("Semantic Analysis Error: Addition between non-numeric or non-identical types\n");
					return NULL;
				}
				case AST_BINARY_SUBTRACT:
				{
					
					break;
				}
				case AST_BINARY_MULTIPLY:
				{
					
					break;
				}
				case AST_BINARY_DIVIDE:
				{
					
					break;
				}
				case AST_BINARY_ISEQUAL:
				{
					
					break;
				}
				case AST_BINARY_NOTEQUAL:
				{
					
					break;
				}
				case AST_BINARY_GREATEROREQUAL:
				{
					
					break;
				}
				case AST_BINARY_LESSOREQUAL:
				{
					
					break;
				}
				default:
					break;
			}
			return NULL;
		}
		case AST_VALUE_TERNARY:
		{
			
			return NULL;
		}
		case AST_VALUE_CONSTRUCT:
		{
			
			return NULL;
		}
		case AST_VALUE_MEMBERACCESS:
		{
			
			return NULL;
		}
		case AST_VALUE_METHODCALL:
		{
			
			return NULL;
		}
		default:
			return NULL;
	}
}

