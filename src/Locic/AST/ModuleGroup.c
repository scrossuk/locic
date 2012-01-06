#include <stdlib.h>

#include <Locic/List.h>
#include <Locic/AST/ModuleGroup.h>

AST_ModuleGroup * AST_MakeModuleGroup(){
	AST_ModuleGroup * moduleGroup = malloc(sizeof(AST_ModuleGroup));
	moduleGroup->modules = Locic_List_Alloc();
	return moduleGroup;
}

