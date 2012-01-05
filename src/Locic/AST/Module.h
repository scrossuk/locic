#ifndef LOCIC_AST_MODULE_H
#define LOCIC_AST_MODULE_H

#include <Locic/List.h>
#include <Locic/AST/ClassDecl.h>
#include <Locic/AST/ClassDef.h>
#include <Locic/AST/Function.h>

typedef struct AST_Module{
	char * name;
	Locic_List * functionDeclarations;
	Locic_List * functionDefinitions;
	Locic_List * classDeclarations;
	Locic_List * classDefinitions;
} AST_Module;

AST_Module * AST_MakeModule(char * name);

AST_Module * AST_ModuleAddFunctionDecl(AST_Module * module, AST_FunctionDecl * functionDecl);

AST_Module * AST_ModuleAddFunctionDef(AST_Module * module, AST_FunctionDef * functionDef);

AST_Module * AST_ModuleAddClassDecl(AST_Module * module, AST_ClassDecl * classDecl);

AST_Module * AST_ModuleAddClassDef(AST_Module * module, AST_ClassDef * classDef);

void AST_PrintModule(AST_Module * module);

#endif
