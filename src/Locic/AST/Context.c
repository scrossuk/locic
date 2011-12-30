#include <stdlib.h>

#include <Locic/List.h>
#include <Locic/AST/Context.h>

AST_Context * AST_MakeContext(){
	AST_Context * context = malloc(sizeof(AST_Context));
	context->modules = Locic_List_Alloc();
	return context;
}

