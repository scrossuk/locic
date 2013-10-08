#ifndef LOCIC_PARSER_LEXER_HPP
#define LOCIC_PARSER_LEXER_HPP

#include <cstdio>
#include <Locic/Parser/Context.hpp>
#include <Locic/Parser/LocationInfo.hpp>
#include <Locic/Parser/Token.hpp>

namespace Locic{

	namespace Parser{
		
		void * LexAlloc(FILE * file, Context * context);
		
		int LexGetToken(Token* token, LocationInfo* locationInfo, void * scanner);
		
		void LexFree(void * scanner);
		
	}

}

#endif
