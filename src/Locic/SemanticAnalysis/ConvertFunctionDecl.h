#ifndef LOCIC_SEMANTICANALYSIS_CONVERTFUNCTIONDECL_H
#define LOCIC_SEMANTICANALYSIS_CONVERTFUNCTIONDECL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/Context.h>

SEM_FunctionDecl * Locic_SemanticAnalysis_ConvertFunctionDecl(Locic_SemanticContext * context, AST_FunctionDecl * functionDecl);

#ifdef __cplusplus
}
#endif

#endif
