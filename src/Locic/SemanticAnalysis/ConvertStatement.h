#ifndef LOCIC_SEMANTICANALYSIS_CONVERTSTATEMENT_H
#define LOCIC_SEMANTICANALYSIS_CONVERTSTATEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/Context.h>

int Locic_SemanticAnalysis_WillStatementReturn(SEM_Statement * statement);

SEM_Statement * Locic_SemanticAnalysis_ConvertStatement(Locic_SemanticContext * context, AST_Statement * statement);

#ifdef __cplusplus
}
#endif

#endif
