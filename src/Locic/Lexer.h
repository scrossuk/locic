#ifndef LOCIC_LEXER_H
#define LOCIC_LEXER_H

#include <stdio.h>
#include <Locic/Token.h>

Locic_TokenValue yylval;

class Locic_Lexer{
	public:
		Locic_Lexer(FILE * file);
			
		~Locic_Lexer();
			
		bool lex(Locic_Token * token);
		
};

#endif
