#ifndef LOCIC_SEMANTICANALYSIS_H
#define LOCIC_SEMANTICANALYSIS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/SemanticContext.h>

SEM_ModuleGroup * Locic_SemanticAnalysis_Run(AST_ModuleGroup * moduleGroup);

SEM_Module * Locic_SemanticAnalysis_ConvertModule(Locic_SemanticContext * context, AST_Module * module);

SEM_Type * Locic_SemanticAnalysis_ConvertType(Locic_SemanticContext * context, AST_Type * type);

SEM_FunctionDecl * Locic_SemanticAnalysis_ConvertFunctionDecl(Locic_SemanticContext * context, AST_FunctionDecl * functionDecl);

SEM_FunctionDef * Locic_SemanticAnalysis_ConvertFunctionDef(Locic_SemanticContext * context, AST_FunctionDef * functionDef);

SEM_Scope * Locic_SemanticAnalysis_ConvertScope(Locic_SemanticContext * context, AST_Scope * scope);

SEM_Var * Locic_SemanticAnalysis_ConvertVar(Locic_SemanticContext * context, AST_Var * var);

SEM_Statement * Locic_SemanticAnalysis_ConvertStatement(Locic_SemanticContext * context, AST_Statement * statement);

SEM_Value * Locic_SemanticAnalysis_ConvertValue(Locic_SemanticContext * context, AST_Value * value);

#ifdef __cplusplus
}
#endif

#endif
