#ifndef LOCIC_AST_MODULEGROUP_H
#define LOCIC_AST_MODULEGROUP_H

#include <Locic/List.h>
#include <Locic/AST/ClassDecl.h>
#include <Locic/AST/ClassDef.h>
#include <Locic/AST/Function.h>

typedef struct AST_ModuleGroup{
	Locic_List * modules;
} AST_ModuleGroup;

AST_ModuleGroup * AST_MakeModuleGroup();

#endif
