#ifndef LOCIC_AST_FILE_H
#define LOCIC_AST_FILE_H

#include <Locic/AST/ClassDecl.h>
#include <Locic/AST/ClassDef.h>
#include <Locic/AST/List.h>

typedef struct AST_File{
	AST_List * classDeclarations;
	AST_List * classDefinitions;
} AST_File;

AST_File * AST_MakeFile();

AST_File * AST_FileAddClassDecl(AST_File * file, AST_ClassDecl * classDecl);

AST_File * AST_FileAddClassDef(AST_File * file, AST_ClassDef * classDef);

#endif
