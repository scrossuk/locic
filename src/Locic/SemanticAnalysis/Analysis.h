#ifndef LOCIC_SEMANTICANALYSIS_ANALYSIS_H
#define LOCIC_SEMANTICANALYSIS_ANALYSIS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Locic/AST.h>
#include <Locic/SEM.h>

SEM_ModuleGroup * Locic_SemanticAnalysis_Run(AST_ModuleGroup * moduleGroup);

#ifdef __cplusplus
}
#endif

#endif
