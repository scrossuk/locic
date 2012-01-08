#include <stdio.h>
#include <Locic/AST.h>
#include <Locic/List.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/Context.h>
#include <Locic/SemanticAnalysis/ConvertStatement.h>

SEM_Scope * Locic_SemanticAnalysis_ConvertScope(Locic_SemanticContext * context, AST_Scope * scope){
	SEM_Scope * semScope = SEM_MakeScope();

	// Add this scope to the context, so that variables can be added to it.
	Locic_SemanticContext_PushScope(context, semScope);
	
	// Go through each syntactic statement, and create a corresponding semantic statement.
	Locic_List * synStatements = scope->statementList;
	Locic_ListElement * it;
	
	for(it = Locic_List_Begin(synStatements); it != Locic_List_End(synStatements); it = it->next){
		SEM_Statement * statement = Locic_SemanticAnalysis_ConvertStatement(context, it->data);
		if(statement == NULL){
			return NULL;
		}
		
		// Add the new statement to the scope.
		Locic_List_Append(semScope->statementList, statement);
	}
	
	// Remove this scope from the context.
	Locic_SemanticContext_PopScope(context);
	
	return semScope;
}


