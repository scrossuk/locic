#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Locic/AST.hpp>
#include <Locic/Parser/DefaultParser.hpp>
#include <Locic/CodeGen/CodeGen.hpp>
#include <Locic/SemanticAnalysis.hpp>

int main(int argc, char * argv[]){
	if(argc < 2){
		printf("Locic: No files provided.\n");
		return 1;
	}
	
	assert(argc >= 1);
	const std::size_t numFiles = argc - 1;
	assert(numFiles > 0);
	
	std::vector<AST::Module *> astModules;
	
	// Parse all source files.
	for(std::size_t i = 0; i < numFiles; i++){
		const std::string filename(argv[1 + i]);
		FILE * file = fopen(filename.c_str(), "rb");
		
		if(file == NULL){
			printf("Parser Error: Failed to open file '%s'.\n", filename.c_str());
			return 1;
		}
		
		Locic::Parser::DefaultParser parser(file, filename);
		if(parser.parseModule()){
			astModules.push_back(parser.getModule());
			fclose(file);
		}else{
			printf("Parser Error: Failed to parse file '%s' with error '%s'.\n", filename.c_str(), parser.getErrorString().c_str());
			return 1;
		}
	}
	
	// Perform semantic analysis.
	std::vector<SEM::Module *> semModules = Locic::SemanticAnalysis::Run(astModules);
	
	if(semModules.empty()){
		printf("Semantic Analysis Failed.\n");
		return 1;
	}
	
	assert(semModules.size() == numFiles);
	
	for(std::size_t i = 0; i < numFiles; i++){
		const std::string moduleName(argv[1 + i]);
		
		void * codeGenContext = Locic_CodeGenAlloc(moduleName);
		Locic_CodeGen(codeGenContext, semModules.at(i));
		Locic_CodeGenWriteToFile(codeGenContext, moduleName + ".ll");
		Locic_CodeGenFree(codeGenContext);
	}
	
	return 0;
}

