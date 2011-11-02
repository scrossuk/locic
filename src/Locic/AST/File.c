#include <stdlib.h>
#include <stdio.h>
#include <Locic/AST/ClassDecl.h>
#include <Locic/AST/ClassDef.h>
#include <Locic/AST/File.h>
#include <Locic/AST/List.h>

AST_File * AST_MakeFile(){
	AST_File * file = malloc(sizeof(AST_File));
	file->classDeclarations = AST_ListCreate();
	file->classDefinitions = AST_ListCreate();
	return file;
}

AST_File * AST_FileAddClassDecl(AST_File * file, AST_ClassDecl * classDecl){
	file->classDeclarations = AST_ListAppend(file->classDeclarations, classDecl);
	return file;
}

AST_File * AST_FileAddClassDef(AST_File * file, AST_ClassDef * classDef){
	file->classDefinitions = AST_ListAppend(file->classDefinitions, classDef);
	return file;
}

void AST_PrintFile(AST_File * file){
	printf("----Class Declarations:\n");
	printf("\n----Class Definitions:\n");
	printf("\n");
}

