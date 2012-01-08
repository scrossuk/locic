#include <assert.h>
#include <stdio.h>
#include <Locic/SemanticAnalysis/SemanticContext.h>

Locic_SemanticContext * Locic_SemanticContext_Alloc(){
	Locic_SemanticContext * context = malloc(sizeof(Locic_SemanticContext));
	context->functionDeclarations = Locic_StringMap_Alloc();
	context->classDeclarations = Locic_StringMap_Alloc();
	
	context->classContext = malloc(sizeof(Locic_SemanticContext_Class));
	context->classContext->memberVariables = Locic_StringMap_Alloc();
	
	context->functionContext = malloc(sizeof(Locic_SemanticContext_Function));
	context->functionContext->parameters = Locic_StringMap_Alloc();
	context->functionContext->nextVarId = 0;
	
	context->scopeStack = Locic_Stack_Alloc();
}

void Locic_SemanticContext_Free(Locic_SemanticContext * context){
	Locic_StringMap_Free(context->functionDeclarations);
	Locic_StringMap_Free(context->classDeclarations);
	
	Locic_StringMap_Free(context->classContext->memberVariables);
	free(context->classContext);
	Locic_StringMap_Free(context->functionContext->parameters);
	free(context->functionContext);
	Locic_StringMap_Free(context->scopeStack);
	free(context);
}

void Locic_SemanticContext_StartFunction(Locic_SemanticContext * context, SEM_ClassDecl * classDecl, SEM_FunctionDecl * functionDecl){
	// Clear class data.
	Locic_StringMap_Clear(context->classContext->memberVariables);
	assert(Locic_Stack_Size(context->scopeStack) == 0);
	
	// Clear function data.
	context->functionContext->nextVarId = 0;
	Locic_StringMap_Clear(context->functionContext->parameters);
	assert(Locic_Stack_Size(context->scopeStack) == 0);
	
	// Set class and function declarations.
	context->classDecl = classDecl;
	context->functionDecl = functionDecl;
}

void Locic_SemanticContext_EndFunction(Locic_SemanticContext * context){
	// Clear function data.
	context->functionContext->nextVarId = 0;
	Locic_StringMap_Clear(context->functionContext->parameters);
	assert(Locic_Stack_Size(context->scopeStack) == 0);
}

void Locic_SemanticContext_PushScope(Locic_SemanticContext * context, SEM_Scope * scope){
	Locic_SemanticContext_Scope * semScope = malloc(sizeof(Locic_SemanticContext_Scope));
	semScope->scope = scope;
	semScope->localVariables = Locic_StringMap_Alloc();
	Locic_Stack_Push(context->scopeStack, semScope);
}

void Locic_SemanticContext_PopScope(Locic_SemanticContext * context){
	Locic_SemanticContext_Scope * semScope = Locic_Stack_Top(context->scopeStack);
	Locic_StringMap_Free(semScope->localVariables);
	free(semScope);
	Locic_Stack_Pop(context->scopeStack);
}

Locic_SemanticContext_Scope * Locic_SemanticContext_TopScope(Locic_SemanticContext * context){
	return (Locic_SemanticContext_Scope *) Locic_Stack_Top(context->scopeStack);
}

SEM_Var * Locic_SemanticContext_DefineLocalVar(Locic_SemanticContext * context, const char * varName, SEM_Type * varType){
	Locic_SemanticContext_Scope * currentScope = Locic_SemanticContext_TopScope(context);
	SEM_Var * semVar = SEM_MakeVar(SEM_VAR_LOCAL, context->functionContext->nextVarId++, varType);
	
	// Add to local variable name map for this scope.
	SEM_Var * existingVar = Locic_StringMap_Insert(currentScope->localVariables, varName, semVar);
	
	if(existingVar == NULL){
		// Add to SEM structure.
		Locic_Array_PushBack(currentScope->scope->localVariables, semVar);
		return semVar;
	}
	
	// Variable already exists => define failed.
	return NULL;
}

SEM_Var * Locic_SemanticContext_FindLocalVar(Locic_SemanticContext * context, const char * varName){
	// Look in the local variables of the current scopes.
	size_t i;
	for(i = Locic_Stack_Size(context->scopeStack); i > 0; i--){
		Locic_SemanticContext_Scope * scope = Locic_Stack_Get(context->scopeStack, i-1);
		SEM_Var * varEntry = Locic_StringMap_Find(scope->localVariables, varName);
		if(varEntry != NULL){
			return varEntry;
		}
	}
	
	// Variable not found in local variables => look in function parameters.
	SEM_Var * varEntry = Locic_StringMap_Find(context->functionContext->parameters, varName);
	if(varEntry != NULL){
		return varEntry;
	}
	
	return NULL;
}

