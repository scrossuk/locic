#include <stdio.h>
#include "Lexer.h"
#include "Parser.h"

int main(int argc, char * argv[]){
	if(argc != 2){
		printf("Locic: No files provided\n");
		return 0;
	}
	
	char * filename = argv[1];

	FILE * file = fopen(filename, "rb");
	
	Lexer lexer(file);
	
	Parser parser;
	
	Token token;
	
	while(lexer.getToken(&token)){
		parser.parse(&token);
	}
	
	fclose(file);
	
	return 0;
}

