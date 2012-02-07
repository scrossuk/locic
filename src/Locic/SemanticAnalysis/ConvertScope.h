#ifndef LOCIC_SEMANTICANALYSIS_CONVERTSCOPE_H
#define LOCIC_SEMANTICANALYSIS_CONVERTSCOPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/Context.h>

int Locic_SemanticAnalysis_WillScopeReturn(SEM_Scope * scope);

SEM_Scope * Locic_SemanticAnalysis_ConvertScope(Locic_SemanticContext * context, AST_Scope * scope);

#ifdef __cplusplus
}
#endif

#endif
