#include <cstdio>
#include <Locic/Lexer.h>

extern FILE * yyin;
extern "C" int yylex();

Locic_Lexer::Locic_Lexer(FILE * file){
	yyin = file;
}
	
Locic_Lexer::~Locic_Lexer(){
	fclose(file_);
}
	
bool Locic_Lexer::lex(Token * token){
	int lexVal = yylex();
	if(lexVal == 0){
		return false;
	}else{
		token->id = lexVal;
		token->value = yylval;
		return true;
	}
}

