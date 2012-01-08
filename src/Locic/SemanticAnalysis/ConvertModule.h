#ifndef LOCIC_SEMANTICANALYSIS_CONVERTMODULE_H
#define LOCIC_SEMANTICANALYSIS_CONVERTMODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Locic/AST.h>
#include <Locic/List.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/Context.h>

SEM_Module * Locic_SemanticAnalysis_ConvertModule(Locic_SemanticContext * context, Locic_List * functionDeclarations, AST_Module * module);

#ifdef __cplusplus
}
#endif

#endif
