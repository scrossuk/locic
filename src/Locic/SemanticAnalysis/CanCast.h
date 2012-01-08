#ifndef LOCIC_SEMANTICANALYSIS_CANCAST_H
#define LOCIC_SEMANTICANALYSIS_CANCAST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/Context.h>

int Locic_SemanticAnalysis_CanDoImplicitCast(Locic_SemanticContext * context, SEM_Type * sourceType, SEM_Type * destType);

int Locic_SemanticAnalysis_CanDoImplicitCopy(Locic_SemanticContext * context, SEM_Type * type);

#ifdef __cplusplus
}
#endif

#endif
