#ifndef LOCIC_SEMANTICANALYSIS_CONVERTVALUE_H
#define LOCIC_SEMANTICANALYSIS_CONVERTVALUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/Context.h>

SEM_Value * Locic_SemanticAnalysis_ConvertValue(Locic_SemanticContext * context, AST_Value * value);

#ifdef __cplusplus
}
#endif

#endif
