#include <stdlib.h>
#include <stdio.h>
#include <Locic/List.h>
#include <Locic/SEM/ClassDecl.h>
#include <Locic/SEM/ClassDef.h>
#include <Locic/SEM/Module.h>

SEM_Module * SEM_MakeModule(char * name){
	SEM_Module * module = malloc(sizeof(SEM_Module));
	module->name = name;
	module->functionDeclarations = Locic_List_Alloc();
	module->functionDefinitions = Locic_List_Alloc();
	module->classDeclarations = Locic_List_Alloc();
	module->classDefinitions = Locic_List_Alloc();
	return module;
}

SEM_Module * SEM_ModuleAddFunctionDecl(SEM_Module * module, SEM_FunctionDecl * functionDecl){
	module->functionDeclarations = Locic_List_Append(module->functionDeclarations, functionDecl);
	return module;
}

SEM_Module * SEM_ModuleAddFunctionDef(SEM_Module * module, SEM_FunctionDef * functionDef){
	module->functionDefinitions = Locic_List_Append(module->functionDefinitions, functionDef);
	return module;
}

SEM_Module * SEM_ModuleAddClassDecl(SEM_Module * module, SEM_ClassDecl * classDecl){
	module->classDeclarations = Locic_List_Append(module->classDeclarations, classDecl);
	return module;
}

SEM_Module * SEM_ModuleAddClassDef(SEM_Module * module, SEM_ClassDef * classDef){
	module->classDefinitions = Locic_List_Append(module->classDefinitions, classDef);
	return module;
}

