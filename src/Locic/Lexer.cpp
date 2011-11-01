#include <cstdio>
#include <Locic/Lexer.h>
#include <Locic/Token.h>

extern FILE * yyin;
extern "C" int yylex();

Locic_Lexer::Locic_Lexer(FILE * file){
	yyin = file;
}
	
Locic_Lexer::~Locic_Lexer(){
	fclose(yyin);
}
	
bool Locic_Lexer::lex(Locic_Token * token){
	int lexVal = yylex();
	if(lexVal == 0){
		return false;
	}else{
		token->id = lexVal;
		token->value = yylval;
		return true;
	}
}

