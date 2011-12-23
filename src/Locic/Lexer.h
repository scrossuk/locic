#ifndef LOCIC_LEXER_H
#define LOCIC_LEXER_H

#include <stdio.h>
#include <Locic/LexerContext.h>

void * Locic_LexAlloc(FILE *, Locic_LexerContext *);
int Locic_Lex(void *);
void Locic_LexFree(void *);

#endif
