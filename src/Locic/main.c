#include <stdio.h>
#include <stdlib.h>
#include <Locic/AST.h>
#include <Locic/Parser.h>
#include <Locic/Token.h>

void * Locic_ParseAlloc(void * (*allocFunc)(size_t));

void Locic_Parse(void * parser, int id, Locic_TokenValue token, AST_File ** resultAST);

void Locic_ParseFree(void * parser, void (*freeFunc)(void *));

void * Locic_alloc(size_t size){
	return malloc(size);
}

void Locic_free(void * ptr){
	free(ptr);
}

FILE * yyin;
int yylex();

extern Locic_TokenValue yylval;

int main(int argc, char * argv[]){
	if(argc < 2){
		printf("Locic: No files provided\n");
		return 0;
	}
	
	char * filename = argv[1];

	FILE * file = fopen(filename, "rb");
	
	yyin = file;
	
	void * parser = Locic_ParseAlloc(Locic_alloc);
	
	AST_File * resultAST = 0;
	
	unsigned int numTokens = 0;
	
	while(1){
		int lexVal = yylex();
		
		printf("lexVal = %d\n", lexVal);
		
		Locic_Parse(parser, lexVal, yylval, &resultAST);
		
		if(lexVal == 0){
			break;
		}
		
		numTokens++;
	}
	
	if(resultAST != 0){
		AST_PrintFile(resultAST);
	}
	
	printf("Used %d tokens\n", numTokens);
	
	Locic_ParseFree(parser, Locic_free);
	
	fclose(file);
	
	return 0;
}

