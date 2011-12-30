#ifndef LOCIC_SEMANTICCONTEXT_H
#define LOCIC_SEMANTICCONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Locic/SEM.h>
#include <Locic/Stack.h>
#include <Locic/StringMap.h>

// Holds information about a scope.
typedef struct Locic_SemanticContext_Scope{
	SEM_Scope * scope;
	StringMap * localVariables;
} Locic_SemanticScope;

typedef struct Locic_SemanticContext_Function{
	StringMap * parameters;
	size_t nextVarId;
} Locic_SemanticScope;

typedef struct Locic_SemanticContext_Class{
	StringMap * memberVariables;
} Locic_SemanticScope;

// Manages conversion from AST to SEM structure.
typedef struct Locic_SemanticContext{
	SEM_ClassDecl * classDecl;
	SEM_FunctionDecl * functionDecl;
	StringMap * functionDeclarations;
	StringMap * classDeclarations;
	Locic_SemanticContext_Class * classContext;
	Locic_SemanticContext_Function * functionContext;
	Stack * scopeStack;
} Locic_SemanticContext;

Locic_SemanticContext * Locic_SemanticContext_Alloc();

void Locic_SemanticContext_Free(Locic_SemanticContext * context);

void Locic_SemanticContext_StartFunction(Locic_SemanticContext * context, SEM_ClassDecl * classDecl, SEM_FunctionDecl * functionDecl);

void Locic_SemanticContext_PushScope(Locic_SemanticContext * context, SEM_Scope * scope);

void Locic_SemanticContext_PopScope(Locic_SemanticContext * context);

Locic_SemanticContext_Scope * Locic_SemanticContext_TopScope(Locic_SemanticContext * context);

SEM_Var * Locic_SemanticContext_DefineLocalVar(Locic_SemanticContext * context, const char * varName, SEM_Type * type);

SEM_Var * Locic_SemanticContext_FindLocalVar(Locic_SemanticContext * context, const char * varName, SEM_Type * type);

#ifdef __cplusplus
}
#endif

#endif
