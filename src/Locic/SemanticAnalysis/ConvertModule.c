#include <stdio.h>
#include <Locic/AST.h>
#include <Locic/List.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/Context.h>
#include <Locic/SemanticAnalysis/ConvertFunctionDef.h>
#include <Locic/SemanticAnalysis/ConvertModule.h>

SEM_Module * Locic_SemanticAnalysis_ConvertModule(Locic_SemanticContext * context, Locic_List * functionDeclarations, AST_Module * module){
	SEM_Module * semModule = SEM_MakeModule(module->name);
	Locic_ListElement * it;
	
	// Add all declarations to each module.
	for(it = Locic_List_Begin(functionDeclarations); it != Locic_List_End(functionDeclarations); it = it->next){
		Locic_List_Append(semModule->functionDeclarations, it->data);
	}
	
	// Build each function definition.
	Locic_List * list = module->functionDefinitions;
	for(it = Locic_List_Begin(list); it != Locic_List_End(list); it = it->next){
		AST_FunctionDef * synFunctionDef = it->data;
		
		SEM_FunctionDef * semFunctionDef = Locic_SemanticAnalysis_ConvertFunctionDef(context, synFunctionDef);
		
		if(semFunctionDef == NULL){
			return NULL;
		}
		
		Locic_List_Append(semModule->functionDefinitions, semFunctionDef);
	}
	
	return semModule;
}

