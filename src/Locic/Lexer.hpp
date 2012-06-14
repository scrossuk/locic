#ifndef LOCIC_LEXER_HPP
#define LOCIC_LEXER_HPP

#include <cstdio>
#include <Locic/LexerContext.hpp>

void * Locic_LexAlloc(FILE *, Locic::LexerContext *);
int Locic_Lex(void *);
void Locic_LexFree(void *);

#endif
