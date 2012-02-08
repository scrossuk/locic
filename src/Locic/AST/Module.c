#include <stdlib.h>
#include <stdio.h>

#include <Locic/List.h>
#include <Locic/AST/ClassDecl.h>
#include <Locic/AST/ClassDef.h>
#include <Locic/AST/Module.h>

AST_Module * AST_MakeModule(char * name){
	AST_Module * module = malloc(sizeof(AST_Module));
	module->name = name;
	module->structs = Locic_List_Alloc();
	module->functionDeclarations = Locic_List_Alloc();
	module->functionDefinitions = Locic_List_Alloc();
	module->classDeclarations = Locic_List_Alloc();
	module->classDefinitions = Locic_List_Alloc();
	return module;
}

AST_Module * AST_ModuleAddStruct(AST_Module * module, AST_Struct * astStruct){
	module->structs = Locic_List_Append(module->structs, astStruct);
	return module;
}

AST_Module * AST_ModuleAddFunctionDecl(AST_Module * module, AST_FunctionDecl * functionDecl){
	module->functionDeclarations = Locic_List_Append(module->functionDeclarations, functionDecl);
	return module;
}

AST_Module * AST_ModuleAddFunctionDef(AST_Module * module, AST_FunctionDef * functionDef){
	module->functionDefinitions = Locic_List_Append(module->functionDefinitions, functionDef);
	return module;
}

AST_Module * AST_ModuleAddClassDecl(AST_Module * module, AST_ClassDecl * classDecl){
	module->classDeclarations = Locic_List_Append(module->classDeclarations, classDecl);
	return module;
}

AST_Module * AST_ModuleAddClassDef(AST_Module * module, AST_ClassDef * classDef){
	module->classDefinitions = Locic_List_Append(module->classDefinitions, classDef);
	return module;
}

void AST_PrintModule(AST_Module * module){
	Locic_ListElement * element;
	printf("----Structs:\n");
	for(element = Locic_List_Begin(module->structs); element != Locic_List_End(module->structs); element = element->next){
		AST_PrintStruct((AST_Struct *) element->data);
	}
	
	printf("----Class Declarations:\n");
	for(element = Locic_List_Begin(module->classDeclarations); element != Locic_List_End(module->classDeclarations); element = element->next){
		AST_PrintClassDecl((AST_ClassDecl *) element->data);
	}
	
	printf("\n----Class Definitions:\n");
	for(element = Locic_List_Begin(module->classDefinitions); element != Locic_List_End(module->classDefinitions); element = element->next){
		AST_PrintClassDef((AST_ClassDef *) element->data);
	}
}

