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
	
	std::vector<std::string> fileNames;
	fileNames.push_back("BuiltInTypes.loci");
	for(std::size_t i = 1; i < argc; i++){
		fileNames.push_back(argv[i]);
	}
	
	std::vector<AST::Module *> astModules;
	
	// Parse all source files.
	for(std::size_t i = 0; i < fileNames.size(); i++){
		const std::string filename = fileNames.at(i);
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
			std::vector<Locic::Parser::Error> errors = parser.getErrors();
			assert(!errors.empty());
		
			printf("Parser Error: Failed to parse file '%s' with %lu errors:\n", filename.c_str(), errors.size());
			
			for(std::size_t i = 0; i < errors.size(); i++){
				const Locic::Parser::Error& error = errors.at(i);
				printf("Parser Error (line %lu): %s\n", error.lineNumber, error.message.c_str());
			}
			
			return 1;
		}
	}
	
	// Perform semantic analysis.
	std::vector<SEM::Module *> semModules = Locic::SemanticAnalysis::Run(astModules);
	
	if(semModules.empty()){
		printf("Semantic Analysis Failed.\n");
		return 1;
	}
	
	assert(semModules.size() == fileNames.size());
	
	for(std::size_t i = 0; i < fileNames.size(); i++){
		const std::string moduleName(fileNames.at(i));
		
		void * codeGenContext = Locic_CodeGenAlloc(moduleName);
		Locic_CodeGen(codeGenContext, semModules.at(i));
		Locic_CodeGenWriteToFile(codeGenContext, moduleName + ".ll");
		Locic_CodeGenFree(codeGenContext);
	}
	
	return 0;
}

