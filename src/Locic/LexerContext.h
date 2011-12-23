#ifndef LOCIC_LEXERCONTEXT_H
#define LOCIC_LEXERCONTEXT_H

#include <stddef.h>
#include <Locic/Token.h>

typedef struct Locic_LexerContext{
	Locic_Token token;
	size_t lineNumber;
} Locic_LexerContext;

#endif
