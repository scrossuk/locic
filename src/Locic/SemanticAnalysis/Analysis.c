#include <assert.h>
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
	
	//-- Create module pairs.
	typedef struct ModulePair{
		AST_Module * astModule;
		SEM_Module * semModule;
	} ModulePair;
	
	Locic_List * modulePairs = Locic_List_Alloc();
	
	Locic_ListElement * moduleIter;
	for(moduleIter = Locic_List_Begin(moduleGroup->modules); moduleIter != Locic_List_End(moduleGroup->modules); moduleIter = moduleIter->next){
		AST_Module * astModule = moduleIter->data;
		assert(astModule != NULL);
		
		ModulePair * pair = malloc(sizeof(ModulePair));
		pair->astModule = astModule;
		pair->semModule = SEM_MakeModule(astModule->name);
		Locic_List_Append(modulePairs, pair);
	}
	
	//-- Initial phase: get all type names.
	Locic_ListElement * pairIt;
	for(pairIt = Locic_List_Begin(modulePairs); pairIt != Locic_List_End(modulePairs); pairIt = pairIt->next){
		ModulePair * pair = pairIt->data;
		AST_Module * synModule = pair->astModule;
		SEM_Module * semModule = pair->semModule;
		
		Locic_SemanticContext_StartModule(context, semModule);
		
		// Look for structures.
		Locic_List * list = synModule->structs;
		Locic_ListElement * iter;
		for(iter = Locic_List_Begin(list); iter != Locic_List_End(list); iter = iter->next){
			AST_Struct * astStruct = iter->data;
			
			SEM_TypeInstance * structType = SEM_MakeStruct(astStruct->name);
			
			if(Locic_StringMap_Insert(context->typeInstances, astStruct->name, structType) != NULL){
				printf("Semantic Analysis Error: type already defined with name '%s'.\n", astStruct->name);
				return NULL;
			}
		}
		
		Locic_SemanticContext_EndModule(context);
	}
	
	//-- Second phase: scan for function and class declarations (so they can be referenced by the final phase).
	for(pairIt = Locic_List_Begin(modulePairs); pairIt != Locic_List_End(modulePairs); pairIt = pairIt->next){
		ModulePair * pair = pairIt->data;
		AST_Module * synModule = pair->astModule;
		SEM_Module * semModule = pair->semModule;
		
		Locic_SemanticContext_StartModule(context, semModule);
		
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
		
		// Look for function definitions (and grab their implicit 'declaration').
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
		
		Locic_SemanticContext_EndModule(context);
	}
	
	//-- In-depth phase: extend the semantic structure with the definitions of functions and class methods.
	SEM_ModuleGroup * semModuleGroup = SEM_MakeModuleGroup();
	
	for(pairIt = Locic_List_Begin(modulePairs); pairIt != Locic_List_End(modulePairs); pairIt = pairIt->next){
		ModulePair * pair = pairIt->data;
		AST_Module * synModule = pair->astModule;
		SEM_Module * semModule = pair->semModule;
		if(Locic_SemanticAnalysis_ConvertModule(context, functionDeclarations, synModule, semModule) == 0){
			return NULL;
		}
		Locic_List_Append(semModuleGroup->modules, semModule);
	}
	
	return semModuleGroup;
}

