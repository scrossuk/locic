#ifndef LOCIC_AST_FILE_H
#define LOCIC_AST_FILE_H

#include <Locic/AST/ClassDecl.h>
#include <Locic/AST/ClassDef.h>
#include <Locic/AST/Function.h>
#include <Locic/AST/List.h>

typedef struct AST_File{
	AST_List * functionDeclarations;
	AST_List * functionDefinitions;
	AST_List * classDeclarations;
	AST_List * classDefinitions;
} AST_File;

AST_File * AST_MakeFile();

AST_File * AST_FileAddFunctionDecl(AST_File * file, AST_FunctionDecl * functionDecl);

AST_File * AST_FileAddFunctionDef(AST_File * file, AST_FunctionDef * functionDef);

AST_File * AST_FileAddClassDecl(AST_File * file, AST_ClassDecl * classDecl);

AST_File * AST_FileAddClassDef(AST_File * file, AST_ClassDef * classDef);

void AST_PrintFile(AST_File * file);

#endif
