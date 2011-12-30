#include <stdlib.h>

#include <Locic/List.h>
#include <Locic/SEM/Context.h>

SEM_Context * SEM_MakeContext(){
	SEM_Context * context = malloc(sizeof(SEM_Context));
	context->modules = Locic_List_Alloc();
	return context;
}

