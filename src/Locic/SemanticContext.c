#include <Locic/SemanticContext.h>

SEM_Var * Locic_SEM_Context_FindLocalVar(const char * localVar){
	
}

Locic_SemanticContext * Locic_SemanticContext_Alloc();

void Locic_SemanticContext_Free(Locic_SemanticContext * context);

void Locic_SemanticContext_PushScope(Locic_SemanticContext * context);

void Locic_SemanticContext_PopScope(Locic_SemanticContext * context);

SEM_Var * Locic_SemanticContext_FindLocalVar(Locic_SemanticContext * context, const char * localVar){
	
}

