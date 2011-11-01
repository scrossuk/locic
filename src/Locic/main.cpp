#include <stdio.h>
#include <stdlib.h>
#include <Locic/Lexer.h>
#include <Locic/Parser.h>

extern "C" void * Locic_ParseAlloc(void * (*allocFunc)(size_t));

extern "C" void Locic_Parse(void * parser, int id, Locic_TokenValue token, void * context);

extern "C" void Locic_ParseFree(void * parser, void (*freeFunc)(void *));

extern "C" void * Locic_alloc(size_t size){
	return malloc(size);
}

extern "C" void Locic_free(void * ptr){
	free(ptr);
}

int main(int argc, char * argv[]){
	if(argc < 2){
		printf("Locic: No files provided\n");
		return 0;
	}
	
	char * filename = argv[1];

	FILE * file = fopen(filename, "rb");
	
	Locic_Lexer lexer(file);
	
	void * parser = Locic_ParseAlloc(Locic_alloc);
	
	Locic_Token token;
	
	while(lexer.lex(&token)){
		Locic_Parse(parser, token.id, token.value, NULL);
	}
	
	Locic_Parse(parser, 0, token.value, NULL);
	
	Locic_ParseFree(parser, Locic_free);
	
	fclose(file);
	
	return 0;
}

