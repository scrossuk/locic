#include <stdio.h>
#include <Locic/AST.h>
#include <Locic/List.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/Context.h>
#include <Locic/SemanticAnalysis/ConvertFunctionDecl.h>
#include <Locic/SemanticAnalysis/ConvertModule.h>

SEM_ModuleGroup * Locic_SemanticAnalysis_Run(AST_ModuleGroup * moduleGroup){
	Locic_SemanticContext * context = Locic_SemanticContext_Alloc();
	
	Locic_List * functionDeclarations = Locic_List_Alloc();
	
	//-- Initial phase: scan for function and class declarations (so they can be referenced by the second phase).
	Locic_ListElement * moduleIter;
	for(moduleIter = Locic_List_Begin(moduleGroup->modules); moduleIter != Locic_List_End(moduleGroup->modules); moduleIter = moduleIter->next){
		AST_Module * synModule = moduleIter->data;
		
		// Look for function declarations.
		Locic_List * list = synModule->functionDeclarations;
		Locic_ListElement * iter;
		for(iter = Locic_List_Begin(list); iter != Locic_List_End(list); iter = iter->next){
			AST_FunctionDecl * synFunctionDecl = iter->data;
			SEM_FunctionDecl * semFunctionDecl = Locic_SemanticAnalysis_ConvertFunctionDecl(context, synFunctionDecl);
			
			if(semFunctionDecl != NULL){
				if(Locic_StringMap_Insert(context->functionDeclarations, semFunctionDecl->name, semFunctionDecl) != NULL){
					printf("Semantic Analysis Error: function already defined with name '%s'.\n", semFunctionDecl->name);
					return NULL;
				}
				Locic_List_Append(functionDeclarations, semFunctionDecl);
			}else{
				return NULL;
			}
		}
		
		// Look for function definitions.
		list = synModule->functionDefinitions;
		for(iter = Locic_List_Begin(list); iter != Locic_List_End(list); iter = iter->next){
			AST_FunctionDef * synFunctionDef = iter->data;
			SEM_FunctionDecl * semFunctionDecl = Locic_SemanticAnalysis_ConvertFunctionDecl(context, synFunctionDef->declaration);
			
			if(semFunctionDecl != NULL){
				if(Locic_StringMap_Insert(context->functionDeclarations, semFunctionDecl->name, semFunctionDecl) != NULL){
					printf("Semantic Analysis Error: function already defined with name '%s'.\n", semFunctionDecl->name);
					return NULL;
				}
				Locic_List_Append(functionDeclarations, semFunctionDecl);
			}else{
				return NULL;
			}
		}
	}
	
	//-- In-depth phase: extend the semantic structure with the definitions of functions and class methods.
	SEM_ModuleGroup * semModuleGroup = SEM_MakeModuleGroup();
	
	for(moduleIter = Locic_List_Begin(moduleGroup->modules); moduleIter != Locic_List_End(moduleGroup->modules); moduleIter = moduleIter->next){
		AST_Module * synModule = moduleIter->data;
		SEM_Module * semModule = Locic_SemanticAnalysis_ConvertModule(context, functionDeclarations, synModule);
		if(semModule == NULL){
			return NULL;
		}
		Locic_List_Append(semModuleGroup->modules, semModule);
	}
	
	return semModuleGroup;
}

