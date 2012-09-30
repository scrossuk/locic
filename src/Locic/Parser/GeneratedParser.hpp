#ifndef LOCIC_PARSER_GENERATEDPARSER_HPP
#define LOCIC_PARSER_GENERATEDPARSER_HPP

#include <cstdio>
#include <string>
#include <Locic/Parser/BisonParser.hpp>
#include <Locic/Parser/Context.hpp>

void * Locic_Parser_ParseAlloc(void * (*allocFunc)(size_t));

void Locic_Parser_Parse(void * parser, int id, Locic::Parser::Token token, Locic::Parser::Context * parserContext);

void Locic_Parser_ParseFree(void * parser, void (*freeFunc)(void *));

void Locic_Parser_ParseTrace(FILE * stream, char * zPrefix);

#endif
