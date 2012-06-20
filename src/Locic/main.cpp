#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Locic/AST.hpp>
#include <Locic/Lexer.hpp>
#include <Locic/LexerContext.hpp>
#include <Locic/Parser.h>
#include <Locic/ParserContext.hpp>
#include <Locic/Token.hpp>
#include <Locic/CodeGen/CodeGen.hpp>
#include <Locic/SemanticAnalysis.hpp>

void * Locic_ParseAlloc(void * (*allocFunc)(size_t));

void Locic_Parse(void * parser, int id, Locic::Token token, Locic::ParserContext * parserContext);

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
	
	std::string filename(argv[1]);

	FILE * file = fopen(filename.c_str(), "rb");
	
	if(file == 0){
		printf("Failed to open file\n");
		return 1;
	}
	
	Locic::LexerContext lexerContext;	
	Locic::ParserContext parserContext(filename);
	
	void * lexer = Locic_LexAlloc(file, &lexerContext);
	void * parser = Locic_ParseAlloc(Locic_alloc);
	
	unsigned int numTokens = 0;
	
	//Locic_ParseTrace(stdout, "==> ");
	
	while(1){
	        int lexVal = Locic_Lex(lexer);
	        
	        parserContext.lineNumber = lexerContext.lineNumber;
	        
	        //printf("Found token at line %d\n", (int) lexerContext.lineNumber);
		
		Locic_Parse(parser, lexVal, lexerContext.token, &parserContext);
		
		if(lexVal == 0){
			break;
		}
		
		numTokens++;
	}
	
	if(!parserContext.parseFailed){
		printf("Performing semantic analysis...\n");

		// Try to do semantic analysis...
		std::list<SEM::Module *> semModules = Locic::SemanticAnalysis::Run(parserContext.modules);
		
		if(!semModules.empty()){
			printf("Successfully performed semantic analysis.\n");
			
			printf("Generating code...\n");
			
			std::string moduleName = std::string(argv[1]) + ".o";
		
			void * codeGenContext = Locic_CodeGenAlloc(moduleName);
			Locic_CodeGen(codeGenContext, semModules.front());
			Locic_CodeGenDump(codeGenContext);
			Locic_CodeGenFree(codeGenContext);
		}else{
			printf("Semantic Analysis failed.\n");
		}
	}else{
		printf("Parsing failed.\n");
	}
	
	printf("Used %d tokens\n", numTokens);
	
	Locic_LexFree(lexer);
	Locic_ParseFree(parser, Locic_free);
	
	fclose(file);
	
	return 0;
}

