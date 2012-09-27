#ifndef LOCIC_PARSER_LEXER_HPP
#define LOCIC_PARSER_LEXER_HPP

#include <cstdio>
#include <Locic/Parser/Context.hpp>

namespace Locic{

	namespace Parser{
		
		void * LexAlloc(FILE *, Context *);
		
		int LexGetToken(void *);
		
		void LexFree(void *);
		
	}

}

#endif
