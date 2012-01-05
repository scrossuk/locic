#include <stdlib.h>

#include <Locic/List.h>
#include <Locic/SEM/ModuleGroup.h>

SEM_ModuleGroup * SEM_MakeModuleGroup(){
	SEM_ModuleGroup * moduleGroup = malloc(sizeof(SEM_ModuleGroup));
	moduleGroup->modules = Locic_List_Alloc();
	return moduleGroup;
}

