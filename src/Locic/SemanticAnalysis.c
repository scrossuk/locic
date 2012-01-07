#include <stdio.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis.h>
#include <Locic/SemanticContext.h>

SEM_ModuleGroup * Locic_SemanticAnalysis_Run(AST_ModuleGroup * moduleGroup){
	Locic_SemanticContext * context = Locic_SemanticContext_Alloc();
	
	Locic_List * functionDeclarations = Locic_List_Alloc();
	
	//-- Initial phase: scan for function and class declarations (so they can be referenced by the second phase).
	Locic_ListElement * moduleIter;
	for(moduleIter = Locic_List_Begin(moduleGroup->modules); moduleIter != Locic_List_End(moduleGroup->modules); moduleIter = moduleIter->next){
		AST_Module * synModule = moduleIter->data;
		
		// Look for function declarations.
		Locic_List * list = synModule->functionDeclarations;
		Locic_ListElement * iter;
		for(iter = Locic_List_Begin(list); iter != Locic_List_End(list); iter = iter->next){
			AST_FunctionDecl * synFunctionDecl = iter->data;
			SEM_FunctionDecl * semFunctionDecl = Locic_SemanticAnalysis_ConvertFunctionDecl(context, synFunctionDecl);
			
			if(semFunctionDecl != NULL){
				if(Locic_StringMap_Insert(context->functionDeclarations, semFunctionDecl->name, semFunctionDecl) != NULL){
					printf("Semantic Analysis Error: function already defined with name '%s'.\n", semFunctionDecl->name);
					return NULL;
				}
				Locic_List_Append(functionDeclarations, semFunctionDecl);
			}else{
				return NULL;
			}
		}
		
		// Look for function definitions.
		list = synModule->functionDefinitions;
		for(iter = Locic_List_Begin(list); iter != Locic_List_End(list); iter = iter->next){
			AST_FunctionDef * synFunctionDef = iter->data;
			SEM_FunctionDecl * semFunctionDecl = Locic_SemanticAnalysis_ConvertFunctionDecl(context, synFunctionDef->declaration);
			
			if(semFunctionDecl != NULL){
				if(Locic_StringMap_Insert(context->functionDeclarations, semFunctionDecl->name, semFunctionDecl) != NULL){
					printf("Semantic Analysis Error: function already defined with name '%s'.\n", semFunctionDecl->name);
					return NULL;
				}
				Locic_List_Append(functionDeclarations, semFunctionDecl);
			}else{
				return NULL;
			}
		}
	}
	
	//-- In-depth phase: extend the semantic structure with the definitions of functions and class methods.
	SEM_ModuleGroup * semModuleGroup = SEM_MakeModuleGroup();
	
	for(moduleIter = Locic_List_Begin(moduleGroup->modules); moduleIter != Locic_List_End(moduleGroup->modules); moduleIter = moduleIter->next){
		AST_Module * synModule = moduleIter->data;
		SEM_Module * semModule = Locic_SemanticAnalysis_ConvertModule(context, functionDeclarations, synModule);
		if(semModule == NULL){
			return NULL;
		}
		Locic_List_Append(semModuleGroup->modules, semModule);
	}
	
	return semModuleGroup;
}

SEM_Module * Locic_SemanticAnalysis_ConvertModule(Locic_SemanticContext * context, Locic_List * functionDeclarations, AST_Module * module){
	SEM_Module * semModule = SEM_MakeModule(module->name);
	Locic_ListElement * it;
	
	// Add all declarations to each module.
	for(it = Locic_List_Begin(functionDeclarations); it != Locic_List_End(functionDeclarations); it = it->next){
		Locic_List_Append(semModule->functionDeclarations, it->data);
	}
	
	// Build each function definition.
	Locic_List * list = module->functionDefinitions;
	for(it = Locic_List_Begin(list); it != Locic_List_End(list); it = it->next){
		AST_FunctionDef * synFunctionDef = it->data;
		
		SEM_FunctionDef * semFunctionDef = Locic_SemanticAnalysis_ConvertFunctionDef(context, synFunctionDef);
		
		if(semFunctionDef == NULL){
			return NULL;
		}
		
		Locic_List_Append(semModule->functionDefinitions, semFunctionDef);
	}
	
	return semModule;
}

SEM_Type * Locic_SemanticAnalysis_ConvertType(Locic_SemanticContext * context, AST_Type * type, SEM_TypeIsLValue isLValue){
	switch(type->typeEnum){
		case AST_TYPE_BASIC:
		{
			return SEM_MakeBasicType(type->isMutable, isLValue, type->basicType.typeEnum);
		}
		case AST_TYPE_NAMED:
		{
			SEM_ClassDecl * classDecl = Locic_StringMap_Find(context->classDeclarations, type->namedType.name);
			if(classDecl == NULL){
				printf("Semantic Analysis Error: Unknown class type with name '%s'.\n", type->namedType.name);
				return NULL;
			}
			
			return SEM_MakeClassType(type->isMutable, isLValue, classDecl);
		}
		case AST_TYPE_PTR:
		{
			// Pointed-to types are always l-values (otherwise they couldn't have their address taken).
			SEM_Type * ptrType = Locic_SemanticAnalysis_ConvertType(context, type->ptrType.ptrType, SEM_TYPE_LVALUE);
			
			if(ptrType == NULL){
				return NULL;
			}
			
			return SEM_MakePtrType(type->isMutable, isLValue, ptrType);
		}
		case AST_TYPE_FUNC:
		{
			SEM_Type * returnType = Locic_SemanticAnalysis_ConvertType(context, type->funcType.returnType, SEM_TYPE_RVALUE);
			if(returnType == NULL){
				return NULL;
			}
			
			Locic_List * parameterTypes = Locic_List_Alloc();
			Locic_ListElement * it;
			for(it = Locic_List_Begin(type->funcType.parameterTypes); it != Locic_List_End(type->funcType.parameterTypes); it = it->next){
				SEM_Type * paramType = Locic_SemanticAnalysis_ConvertType(context, it->data, SEM_TYPE_LVALUE);
				if(paramType == NULL){
					return NULL;
				}
				Locic_List_Append(parameterTypes, paramType);
			}
			return SEM_MakeFuncType(type->isMutable, isLValue, returnType, parameterTypes);
		}
		default:
			printf("Internal Compiler Error: Unknown AST_Type type enum.\n");
			return NULL;
	}
}

SEM_FunctionDecl * Locic_SemanticAnalysis_ConvertFunctionDecl(Locic_SemanticContext * context, AST_FunctionDecl * functionDecl){
	AST_Type * returnType = functionDecl->returnType;
	SEM_Type * semReturnType = Locic_SemanticAnalysis_ConvertType(context, returnType, SEM_TYPE_RVALUE);
	
	if(semReturnType == NULL){
		return NULL;
	}
	
	// Return values are always R-values.
	semReturnType->isLValue = SEM_TYPE_RVALUE;
	
	size_t id = 0;
	Locic_List * parameterVars = Locic_List_Alloc();
	Locic_List * parameterTypes = Locic_List_Alloc();
	
	Locic_ListElement * it;
	for(it = Locic_List_Begin(functionDecl->parameters); it != Locic_List_End(functionDecl->parameters); it = it->next, id++){
		AST_TypeVar * typeVar = it->data;
		AST_Type * paramType = typeVar->type;
		SEM_Type * semParamType = Locic_SemanticAnalysis_ConvertType(context, paramType, SEM_TYPE_LVALUE);
		
		if(semParamType == NULL){
			return NULL;
		}
		
		SEM_Var * semParamVar = SEM_MakeVar(SEM_VAR_PARAM, id, semParamType);
		
		Locic_List_Append(parameterTypes, semParamType);
		Locic_List_Append(parameterVars, semParamVar);
	}
	
	SEM_Type * functionType = SEM_MakeFuncType(SEM_TYPE_MUTABLE, SEM_TYPE_RVALUE, semReturnType, parameterTypes);
	
	return SEM_MakeFunctionDecl(functionType, functionDecl->name, parameterVars);
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
				printf("Semantic Analysis Error: cannot convert between incompatible class types.\n");
				return 0;
			}
			
			// Check for const-correctness.
			if(sourceType->isMutable == SEM_TYPE_CONST && destType->isMutable == SEM_TYPE_MUTABLE){
				printf("Semantic Analysis Error: Const-correctness violation.\n");
				return 0;
			}
			
			if(sourceType->isLValue != destType->isLValue){
				printf("Semantic Analysis Error: cannot convert between l-values and r-values.\n");
				return 0;
			}
			
			return 1;
		}
		case SEM_TYPE_PTR:
		{
			// Check for const-correctness.
			if(sourceType->ptrType.ptrType->isMutable == SEM_TYPE_CONST && destType->ptrType.ptrType->isMutable == SEM_TYPE_MUTABLE){
				printf("Semantic Analysis Error: Const-correctness violation on pointer type.\n");
				return 0;
			}
			
			if(!Locic_SemanticAnalysis_CanDoImplicitCast(context, sourceType->ptrType.ptrType, destType->ptrType.ptrType)){
				return 0;
			}
			return 1;
		}
		case SEM_TYPE_FUNC:
		{
			if(!Locic_SemanticAnalysis_CanDoImplicitCast(context, sourceType->funcType.returnType, destType->funcType.returnType)){
				return 0;
			}
			
			Locic_List * sourceList = sourceType->funcType.parameterTypes;
			Locic_List * destList = destType->funcType.parameterTypes;
			
			if(Locic_List_Size(sourceList) != Locic_List_Size(destList)){
				return 0;
			}
			
			Locic_ListElement * sourceIt = Locic_List_Begin(sourceList);
			Locic_ListElement * destIt = Locic_List_Begin(destList);
			
			while(sourceIt != Locic_List_End(sourceList)){
				if(!Locic_SemanticAnalysis_CanDoImplicitCast(context, sourceIt->data, destIt->data)){
					return 0;
				}
				sourceIt = sourceIt->next;
				destIt = destIt->next;
			}
			
			return 1;
		}
		default:
			return 0;
	}
}

int Locic_SemanticAnalysis_CanDoImplicitCopy(Locic_SemanticContext * context, SEM_Type * type){
	switch(type->typeEnum){
		case SEM_TYPE_BASIC:
		case SEM_TYPE_PTR:
		case SEM_TYPE_FUNC:
			// Basic, pointer and function types can be copied implicitly.
			return 1;
		default:
			return 0;
	}
}

SEM_FunctionDef * Locic_SemanticAnalysis_ConvertFunctionDef(Locic_SemanticContext * context, AST_FunctionDef * functionDef){
	// Find the corresponding semantic function declaration.
	SEM_FunctionDecl * semFunctionDecl = Locic_StringMap_Find(context->functionDeclarations, functionDef->declaration->name);
	if(semFunctionDecl == NULL){
		printf("Internal compiler error: semantic function declaration not found for definition '%s'.\n", functionDef->declaration->name);
		return NULL;
	}
	
	Locic_SemanticContext_StartFunction(context, NULL, semFunctionDecl);
	
	// AST information gives parameter names; SEM information gives parameter variable information.
	Locic_List * synParameters = functionDef->declaration->parameters;
	Locic_List * semParameters = semFunctionDecl->parameterVars;
	
	Locic_ListElement * synIterator, * semIterator;
	for(synIterator = Locic_List_Begin(synParameters), semIterator = Locic_List_Begin(semParameters);
		synIterator != Locic_List_End(synParameters);
		synIterator = synIterator->next, semIterator = semIterator->next){
		
		AST_TypeVar * typeVar = synIterator->data;
		SEM_Var * paramVar = semIterator->data;
		
		// Create a mapping from the parameter's name to its variable information.
		if(Locic_StringMap_Insert(context->functionContext->parameters, typeVar->name, paramVar) != NULL){
			printf("Semantic Analysis Error: cannot share names between function parameters.\n");
			return NULL;
		}
	}
	
	// Generate the outer function scope.
	SEM_Scope * scope = Locic_SemanticAnalysis_ConvertScope(context, functionDef->scope);
	
	if(scope == NULL){
		return NULL;
	}
	
	Locic_SemanticContext_EndFunction(context);
	
	// Build and return the function definition.
	return SEM_MakeFunctionDef(semFunctionDecl, scope);
}

SEM_Scope * Locic_SemanticAnalysis_ConvertScope(Locic_SemanticContext * context, AST_Scope * scope){
	SEM_Scope * semScope = SEM_MakeScope();

	// Add this scope to the context, so that variables can be added to it.
	Locic_SemanticContext_PushScope(context, semScope);
	
	// Go through each syntactic statement, and create a corresponding semantic statement.
	Locic_List * synStatements = scope->statementList;
	Locic_ListElement * it;
	
	for(it = Locic_List_Begin(synStatements); it != Locic_List_End(synStatements); it = it->next){
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
			printf("Internal Compiler Error: Unimplemented IF statement.\n");
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

SEM_Value * Locic_SemanticAnalysis_ConvertValue(Locic_SemanticContext * context, AST_Value * value){
	if(value == NULL){
		printf("Internal compiler error: Cannot convert NULL AST_Value.\n");
		return NULL;
	}

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
					printf("Internal Compiler Error: Unknown constant type enum.\n");
					return NULL;
			}
		}
		case AST_VALUE_VAR:
		{
			AST_Var * synVar = value->varValue.var;
			switch(synVar->type){
				case AST_VAR_LOCAL:
				{
					SEM_Var * semVar = Locic_SemanticContext_FindLocalVar(context, synVar->localVar.name);
					if(semVar != NULL){
						return SEM_MakeVarValue(semVar);
					}
					
					// Not a variable - try looking for functions.
					SEM_FunctionDecl * decl = Locic_StringMap_Find(context->functionDeclarations, synVar->localVar.name);
					
					if(decl != NULL){
						return SEM_MakeFunctionRef(decl, decl->type);
					}
					
					printf("Semantic Analysis Error: Local variable '%s' was not found\n", synVar->localVar.name);
					return NULL; 
				}
				case AST_VAR_THIS:
				{
					printf("Semantic Analysis Error: Member variables not implemented.\n");
					return NULL;
				}
				default:
					printf("Internal Compiler Error: Unknown AST_Var type enum.\n");
					return NULL;
			}
		}
		case AST_VALUE_UNARY:
		{
			SEM_Value * operand = Locic_SemanticAnalysis_ConvertValue(context, value->unary.value);
			if(operand == NULL){
				return NULL;
			}
			
			switch(value->unary.type){
				case AST_UNARY_PLUS:
				{
					if(operand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_Type * typeCopy = SEM_CopyType(operand->type);
						typeCopy->isMutable = SEM_TYPE_MUTABLE;
						typeCopy->isLValue = SEM_TYPE_RVALUE;
						SEM_BasicTypeEnum basicType = typeCopy->basicType.typeEnum;
						if(basicType == SEM_TYPE_BASIC_INT){
							return SEM_MakeUnary(SEM_UNARY_PLUS, SEM_OP_INT, operand, typeCopy);
						}else if(basicType == SEM_TYPE_BASIC_FLOAT){
							return SEM_MakeUnary(SEM_UNARY_PLUS, SEM_OP_FLOAT, operand, typeCopy);
						}
					}
					printf("Semantic Analysis Error: Unary plus on non-numeric type.\n");
					return NULL;
				}
				case AST_UNARY_MINUS:
				{
					if(operand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_Type * typeCopy = SEM_CopyType(operand->type);
						typeCopy->isMutable = SEM_TYPE_MUTABLE;
						typeCopy->isLValue = SEM_TYPE_RVALUE;
						SEM_BasicTypeEnum basicType = typeCopy->basicType.typeEnum;
						if(basicType == SEM_TYPE_BASIC_INT){
							return SEM_MakeUnary(SEM_UNARY_MINUS, SEM_OP_INT, operand, typeCopy);
						}else if(basicType == SEM_TYPE_BASIC_FLOAT){
							return SEM_MakeUnary(SEM_UNARY_MINUS, SEM_OP_FLOAT, operand, typeCopy);
						}
					}
					printf("Semantic Analysis Error: Unary minus on non-numeric type.\n");
					return NULL;
				}
				case AST_UNARY_ADDRESSOF:
				{
					if(operand->type->isLValue == SEM_TYPE_LVALUE){
						return SEM_MakeUnary(SEM_UNARY_ADDRESSOF, SEM_OP_PTR, operand, SEM_MakePtrType(SEM_TYPE_MUTABLE, SEM_TYPE_RVALUE, operand->type));
					}
					
					printf("Semantic Analysis Error: Attempting to take address of R-value.\n");
					return NULL;
				}
				case AST_UNARY_DEREF:
				{
					if(operand->type->typeEnum == SEM_TYPE_PTR){
						return SEM_MakeUnary(SEM_UNARY_DEREF, SEM_OP_PTR, operand, operand->type->ptrType.ptrType);
					}
					
					printf("Semantic Analysis Error: Attempting to dereference non-pointer type.\n");
					return NULL;
				}
				case AST_UNARY_NOT:
				{
					if(operand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_Type * typeCopy = SEM_CopyType(operand->type);
						typeCopy->isMutable = SEM_TYPE_MUTABLE;
						typeCopy->isLValue = SEM_TYPE_RVALUE;
						if(typeCopy->basicType.typeEnum == SEM_TYPE_BASIC_BOOL){
							return SEM_MakeUnary(SEM_UNARY_NOT, SEM_OP_BOOL, operand, typeCopy);
						}
					}
					
					printf("Semantic Analysis Error: Unary NOT on non-bool type.\n");
					return NULL;
				}
				default:
					printf("Internal Compiler Error: Unknown unary value type enum.\n");
					return NULL;
			}
		}
		case AST_VALUE_BINARY:
		{
			SEM_Value * leftOperand, * rightOperand;
			leftOperand = Locic_SemanticAnalysis_ConvertValue(context, value->binary.left);
			rightOperand = Locic_SemanticAnalysis_ConvertValue(context, value->binary.right);
			if(leftOperand == NULL || rightOperand == NULL){
				return NULL;
			}
			
			switch(value->binary.type){
				case AST_BINARY_ADD:
				{
					if(leftOperand->type->typeEnum == SEM_TYPE_BASIC && rightOperand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum leftBasicType, rightBasicType;
						leftBasicType = leftOperand->type->basicType.typeEnum;
						rightBasicType = rightOperand->type->basicType.typeEnum;
						
						if(leftBasicType == rightBasicType){
							SEM_Type * typeCopy = SEM_CopyType(leftOperand->type);
							typeCopy->isLValue = SEM_TYPE_RVALUE;
							
							if(leftBasicType == SEM_TYPE_BASIC_INT){
								return SEM_MakeBinary(SEM_BINARY_ADD, SEM_OP_INT, leftOperand, rightOperand, typeCopy);
							}else if(leftBasicType == SEM_TYPE_BASIC_FLOAT){
								return SEM_MakeBinary(SEM_BINARY_ADD, SEM_OP_FLOAT, leftOperand, rightOperand, typeCopy);
							}
						}
					}
					printf("Semantic Analysis Error: Addition between non-numeric or non-identical types.\n");
					return NULL;
				}
				case AST_BINARY_SUBTRACT:
				{
					if(leftOperand->type->typeEnum == SEM_TYPE_BASIC && rightOperand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum leftBasicType, rightBasicType;
						leftBasicType = leftOperand->type->basicType.typeEnum;
						rightBasicType = rightOperand->type->basicType.typeEnum;
						
						if(leftBasicType == rightBasicType){
							SEM_Type * typeCopy = SEM_CopyType(leftOperand->type);
							typeCopy->isLValue = SEM_TYPE_RVALUE;
							
							if(leftBasicType == SEM_TYPE_BASIC_INT){
								return SEM_MakeBinary(SEM_BINARY_SUBTRACT, SEM_OP_INT, leftOperand, rightOperand, typeCopy);
							}else if(leftBasicType == SEM_TYPE_BASIC_FLOAT){
								return SEM_MakeBinary(SEM_BINARY_SUBTRACT, SEM_OP_FLOAT, leftOperand, rightOperand, typeCopy);
							}
						}
					}
					printf("Semantic Analysis Error: Subtraction between non-numeric or non-identical types.\n");
					return NULL;
				}
				case AST_BINARY_MULTIPLY:
				{
					if(leftOperand->type->typeEnum == SEM_TYPE_BASIC && rightOperand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum leftBasicType, rightBasicType;
						leftBasicType = leftOperand->type->basicType.typeEnum;
						rightBasicType = rightOperand->type->basicType.typeEnum;
						
						if(leftBasicType == rightBasicType){
							SEM_Type * typeCopy = SEM_CopyType(leftOperand->type);
							typeCopy->isLValue = SEM_TYPE_RVALUE;
							
							if(leftBasicType == SEM_TYPE_BASIC_INT){
								return SEM_MakeBinary(SEM_BINARY_MULTIPLY, SEM_OP_INT, leftOperand, rightOperand, typeCopy);
							}else if(leftBasicType == SEM_TYPE_BASIC_FLOAT){
								return SEM_MakeBinary(SEM_BINARY_MULTIPLY, SEM_OP_FLOAT, leftOperand, rightOperand, typeCopy);
							}
						}
					}
					printf("Semantic Analysis Error: Multiplication between non-numeric or non-identical types.\n");
					return NULL;
				}
				case AST_BINARY_DIVIDE:
				{
					if(leftOperand->type->typeEnum == SEM_TYPE_BASIC && rightOperand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum leftBasicType, rightBasicType;
						leftBasicType = leftOperand->type->basicType.typeEnum;
						rightBasicType = rightOperand->type->basicType.typeEnum;
						
						if(leftBasicType == rightBasicType){
							SEM_Type * typeCopy = SEM_CopyType(leftOperand->type);
							typeCopy->isLValue = SEM_TYPE_RVALUE;
							
							if(leftBasicType == SEM_TYPE_BASIC_INT){
								return SEM_MakeBinary(SEM_BINARY_DIVIDE, SEM_OP_INT, leftOperand, rightOperand, typeCopy);
							}else if(leftBasicType == SEM_TYPE_BASIC_FLOAT){
								return SEM_MakeBinary(SEM_BINARY_DIVIDE, SEM_OP_FLOAT, leftOperand, rightOperand, typeCopy);
							}
						}
					}
					printf("Semantic Analysis Error: Division between non-numeric or non-identical types.\n");
					return NULL;
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
					printf("Internal Compiler Error: Unknown binary value type enum.\n");
					return NULL;
			}
			printf("Internal Compiler Error: Unimplemented binary operator.\n");
			return NULL;
		}
		case AST_VALUE_TERNARY:
		{
			printf("Internal Compiler Error: Unimplemented ternary operator.\n");
			return NULL;
		}
		case AST_VALUE_CONSTRUCT:
		{
			printf("Internal Compiler Error: Unimplemented constructor call.\n");
			return NULL;
		}
		case AST_VALUE_MEMBERACCESS:
		{
			printf("Internal Compiler Error: Unimplemented member access.\n");
			return NULL;
		}
		case AST_VALUE_FUNCTIONCALL:
		{
			SEM_Value * functionValue = Locic_SemanticAnalysis_ConvertValue(context, value->functionCall.functionValue);
			
			if(functionValue == NULL){
				return NULL;
			}
			
			if(functionValue->type->typeEnum != SEM_TYPE_FUNC){
				printf("Semantic Analysis Error: Can't call non-function type.\n");
				return NULL;
			}
			
			Locic_List * typeList = functionValue->type->funcType.parameterTypes;
			Locic_List * synValueList = value->functionCall.parameters;
			
			if(Locic_List_Size(typeList) != Locic_List_Size(synValueList)){
				printf("Semantic Analysis Error: Function called with %lu number of parameters; expected %lu.\n", Locic_List_Size(synValueList), Locic_List_Size(typeList));
				return NULL;
			}
			
			Locic_List * semValueList = Locic_List_Alloc();
			
			Locic_ListElement * typeIt = Locic_List_Begin(typeList);
			Locic_ListElement * valueIt = Locic_List_Begin(synValueList);
			
			while(valueIt != Locic_List_End(synValueList)){
				SEM_Value * param = Locic_SemanticAnalysis_ConvertValue(context, valueIt->data);
				
				if(param == NULL){
					return NULL;
				}
			
				if(!Locic_SemanticAnalysis_CanDoImplicitCast(context, param->type, typeIt->data)){
					printf("Semantic Analysis Error: Cannot convert parameter value to type expected by function.\n");
					return NULL;
				}
				
				Locic_List_Append(semValueList, param);
				
				typeIt = typeIt->next;
				valueIt = valueIt->next;
			}
			
			return SEM_MakeFunctionCall(functionValue, semValueList, functionValue->type->funcType.returnType);
		}
		default:
			printf("Internal Compiler Error: Unknown AST_Value type enum.\n");
			return NULL;
	}
}

