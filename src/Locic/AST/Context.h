#ifndef LOCIC_AST_CONTEXT_H
#define LOCIC_AST_CONTEXT_H

#include <Locic/List.h>
#include <Locic/AST/ClassDecl.h>
#include <Locic/AST/ClassDef.h>
#include <Locic/AST/Function.h>

typedef struct AST_Context{
	Locic_List * modules;
} AST_Context;

AST_Context * AST_MakeContext();

#endif
