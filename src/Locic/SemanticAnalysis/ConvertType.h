#ifndef LOCIC_SEMANTICANALYSIS_CONVERTTYPE_H
#define LOCIC_SEMANTICANALYSIS_CONVERTTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/Context.h>

// Convert a type annotation to a semantic type definition.
SEM_Type * Locic_SemanticAnalysis_ConvertType(Locic_SemanticContext * context, AST_Type * type, SEM_TypeIsLValue isLValue);

#ifdef __cplusplus
}
#endif

#endif
