#ifndef LOCIC_PARSERCONTEXT_H
#define LOCIC_PARSERCONTEXT_H

#include <stddef.h>
#include <Locic/AST.h>

typedef struct Locic_ParserContext{
	AST_Context * synContext;
	const char * currentFileName;
	size_t lineNumber;
	int parseFailed;
} Locic_ParserContext;

#endif
