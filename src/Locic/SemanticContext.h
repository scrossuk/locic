#ifndef LOCIC_SEMANTICCONTEXT_H
#define LOCIC_SEMANTICCONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Locic/SEM.h>
#include <Locic/Stack.h>
#include <Locic/StringMap.h>

typedef struct Locic_SemanticScope{
	StringMap * memberVariables;
	StringMap * localVariables;
} Locic_SemanticScope;

typedef struct Locic_SemanticContext{
	StringMap * functionDeclarations;
	StringMap * classDeclarations;
	Stack * scopeStack;
} Locic_SemanticContext;

Locic_SemanticContext * Locic_SemanticContext_Alloc();

void Locic_SemanticContext_Free(Locic_SemanticContext * context);

inline void Locic_SemanticContext_PushScope(Locic_SemanticContext * context){
	
}

inline void Locic_SemanticContext_PopScope(Locic_SemanticContext * context){
	
}

inline Locic_SemanticScope * Locic_SemanticContext_TopScope(Locic_SemanticContext * context){
	return (Locic_SemanticScope *) Locic_Stack_Top(context->scopeStack);
}

SEM_Var * Locic_SemanticContext_FindLocalVar(Locic_SemanticContext * context, const char * localVar);

#ifdef __cplusplus
}
#endif

#endif
