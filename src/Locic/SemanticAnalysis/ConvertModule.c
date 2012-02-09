#include <stdio.h>
#include <Locic/AST.h>
#include <Locic/List.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/Context.h>
#include <Locic/SemanticAnalysis/ConvertFunctionDef.h>
#include <Locic/SemanticAnalysis/ConvertModule.h>

int Locic_SemanticAnalysis_ConvertModule(Locic_SemanticContext * context, Locic_List * functionDeclarations, AST_Module * module, SEM_Module * semModule){
	Locic_ListElement * it;
	
	Locic_SemanticContext_StartModule(context, semModule);
	
	// Build each function definition.
	Locic_List * list = module->functionDefinitions;
	for(it = Locic_List_Begin(list); it != Locic_List_End(list); it = it->next){
		AST_FunctionDef * synFunctionDef = it->data;
		
		SEM_FunctionDef * semFunctionDef = Locic_SemanticAnalysis_ConvertFunctionDef(context, synFunctionDef);
		
		if(semFunctionDef == NULL){
			return 0;
		}
		
		Locic_List_Append(semModule->functionDefinitions, semFunctionDef);
	}
	
	Locic_SemanticContext_EndModule(context);
	
	return 1;
}

