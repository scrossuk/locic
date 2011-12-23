#include <stdio.h>
#include <stdlib.h>
#include <Locic/AST.h>
#include <Locic/Lexer.h>
#include <Locic/LexerContext.h>
#include <Locic/Parser.h>
#include <Locic/Token.h>

void * Locic_ParseAlloc(void * (*allocFunc)(size_t));

void Locic_Parse(void * parser, int id, Locic_Token token, AST_File ** resultAST);

void Locic_ParseFree(void * parser, void (*freeFunc)(void *));

void Locic_ParseTrace(FILE * stream, char * zPrefix);

void * Locic_alloc(size_t size){
	return malloc(size);
}

void Locic_free(void * ptr){
	free(ptr);
}

int main(int argc, char * argv[]){
	if(argc < 2){
		printf("Locic: No files provided\n");
		return 0;
	}
	
	char * filename = argv[1];

	FILE * file = fopen(filename, "rb");
	
	if(file == 0){
		printf("Failed to open file\n");
		return 1;
	}
	
	Locic_LexerContext lexerContext;
	void * lexer = Locic_LexAlloc(file, &lexerContext);
	void * parser = Locic_ParseAlloc(Locic_alloc);
	
	AST_File * resultAST = 0;
	
	unsigned int numTokens = 0;
	
	//Locic_ParseTrace(stdout, "==> ");
	
	while(1){
	        int lexVal = Locic_Lex(lexer);
	        
	        //printf("Found token at line %d\n", (int) lexerContext.lineNumber);
		
		Locic_Parse(parser, lexVal, lexerContext.token, &resultAST);
		
		if(lexVal == 0){
			break;
		}
		
		numTokens++;
	}
	
	if(resultAST != 0){
		AST_PrintFile(resultAST);
	}
	
	printf("Used %d tokens\n", numTokens);
	
	Locic_LexFree(lexer);
	Locic_ParseFree(parser, Locic_free);
	
	fclose(file);
	
	return 0;
}

