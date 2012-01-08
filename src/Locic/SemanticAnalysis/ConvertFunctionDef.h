#ifndef LOCIC_SEMANTICANALYSIS_CONVERTFUNCTIONDEF_H
#define LOCIC_SEMANTICANALYSIS_CONVERTFUNCTIONDEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/Context.h>

SEM_FunctionDef * Locic_SemanticAnalysis_ConvertFunctionDef(Locic_SemanticContext * context, AST_FunctionDef * functionDef);

#ifdef __cplusplus
}
#endif

#endif
