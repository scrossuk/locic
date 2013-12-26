#ifndef LOCIC_PARSER_LEXER_HPP
#define LOCIC_PARSER_LEXER_HPP

#include <cstdio>

#include <locic/Parser/Context.hpp>

#include "LocationInfo.hpp"
#include "Token.hpp"

namespace locic{

	namespace Parser{
		
		void * LexAlloc(FILE * file, Context * context);
		
		int LexGetToken(Token* token, LocationInfo* locationInfo, void * scanner);
		
		void LexFree(void * scanner);
		
	}

}

#endif
