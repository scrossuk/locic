#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_H
#define LOCIC_SEMANTICANALYSIS_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Locic/SEM.h>
#include <Locic/Stack.h>
#include <Locic/StringMap.h>

// Holds information about a scope during its conversion.
typedef struct Locic_SemanticContext_Scope{
	// The resulting semantic scope that will be generated.
	SEM_Scope * scope;
	
	// Map from local variable names to their semantic variables.
	Locic_StringMap localVariables;
} Locic_SemanticContext_Scope;

// Holds information about a function during its conversion.
typedef struct Locic_SemanticContext_Function{
	// Map from parameter variable names to their semantic variables.
	Locic_StringMap * parameters;
	
	// The id to be assigned to the next local variable.
	size_t nextVarId;
} Locic_SemanticContext_Function;

// Holds information about a class during its conversion.
typedef struct Locic_SemanticContext_Class{
	Locic_StringMap * memberVariables;
} Locic_SemanticContext_Class;

// Manages conversion from AST to SEM structure.
typedef struct Locic_SemanticContext{
	// The current class declaration.
	// (NULL if current function is not a class method.)
	SEM_ClassDecl * classDecl;
	
	// The current function declaration.
	SEM_FunctionDecl * functionDecl;
	
	// Map from function names to their declarations for all modules.
	Locic_StringMap functionDeclarations;
	
	// Map from class names to their declarations for all modules.
	Locic_StringMap classDeclarations;
	
	// Information about the class containing the current function.
	// (e.g. member variables).
	Locic_SemanticContext_Class classContext;
	
	// Information about the current function.
	// (e.g. parameters).
	Locic_SemanticContext_Function functionContext;
	
	// A stack of scope contexts.
	Locic_Stack * scopeStack;
} Locic_SemanticContext;

Locic_SemanticContext * Locic_SemanticContext_Alloc();

void Locic_SemanticContext_Free(Locic_SemanticContext * context);

void Locic_SemanticContext_StartClass(Locic_SemanticContext * context, SEM_ClassDecl * classDecl);

void Locic_SemanticContext_EndClass(Locic_SemanticContext * context);

void Locic_SemanticContext_StartFunction(Locic_SemanticContext * context, SEM_FunctionDecl * functionDecl);

void Locic_SemanticContext_EndFunction(Locic_SemanticContext * context);

void Locic_SemanticContext_PushScope(Locic_SemanticContext * context, SEM_Scope * scope);

void Locic_SemanticContext_PopScope(Locic_SemanticContext * context);

Locic_SemanticContext_Scope * Locic_SemanticContext_TopScope(Locic_SemanticContext * context);

SEM_Var * Locic_SemanticContext_DefineLocalVar(Locic_SemanticContext * context, const char * varName, SEM_Type * type);

SEM_Var * Locic_SemanticContext_FindLocalVar(Locic_SemanticContext * context, const char * varName);

#ifdef __cplusplus
}
#endif

#endif
