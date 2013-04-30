#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>

#include <Locic/AST.hpp>
#include <Locic/Parser/DefaultParser.hpp>
#include <Locic/CodeGen/CodeGen.hpp>
#include <Locic/SemanticAnalysis.hpp>

using namespace Locic;

int main(int argc, char * argv[]){
	if(argc < 2){
		printf("Locic: No files provided.\n");
		return 1;
	}
	assert(argc >= 1);
	
	size_t optLevel = 0;
	
	std::vector<std::string> fileNames;
	fileNames.push_back("BuiltInTypes.loci");
	
	for(size_t i = 1; i < static_cast<size_t>(argc); i++){
		const std::string argument(argv[i]);
		
		assert(!argument.empty());
	
		if(argument.at(0) == '-'){
			if(argument.size() == 1){
				printf("Locic: Invalid option specifier.\n");
				return -1;
			}
			if(argument.at(1) == 'O'){
				if(argument.size() != 3){
					printf("Locic: Invalid option '%s'.\n", argument.c_str());
					return -1;
				}
				switch(argument.at(2)){
					case '0':
						optLevel = 0;
						break;
					case '1':
						optLevel = 1;
						break;
					case '2':
						optLevel = 2;
						break;
					case '3':
						optLevel = 3;
						break;
					default:
						printf("Locic: Invalid optimisation level '%c'.\n", argument.at(2));
						return -1;
				}
			}else{
				printf("Locic: Unknown option '%s'.\n", argument.c_str());
				return -1;
			}
		}else{
			fileNames.push_back(argument);
		}
	}
	
	AST::Namespace * rootASTNamespace = new AST::Namespace("");
	
	// Parse all source files.
	for(std::size_t i = 0; i < fileNames.size(); i++){
		const std::string filename = fileNames.at(i);
		FILE * file = fopen(filename.c_str(), "rb");
		
		if(file == NULL){
			printf("Parser Error: Failed to open file '%s'.\n", filename.c_str());
			return 1;
		}
		
		Parser::DefaultParser parser(rootASTNamespace, file, filename);
		if(!parser.parseFile()){
			std::vector<Parser::Error> errors = parser.getErrors();
			assert(!errors.empty());
		
			printf("Parser Error: Failed to parse file '%s' with %lu errors:\n", filename.c_str(), errors.size());
			
			for(size_t j = 0; j < errors.size(); j++){
				const Parser::Error& error = errors.at(j);
				printf("Parser Error (line %lu): %s\n", error.lineNumber, error.message.c_str());
			}
			
			return 1;
		}
	}
	
	// Perform semantic analysis.
	SEM::Namespace * rootSEMNamespace = SemanticAnalysis::Run(rootASTNamespace);
	assert(rootSEMNamespace != NULL);
	
	{
		std::ofstream ofs("sem.txt", std::ios_base::binary);
		ofs << formatMessage(rootSEMNamespace->toString());
	}
	
	const std::string outputName = "output";
	
	CodeGen::TargetInfo targetInfo = CodeGen::TargetInfo::DefaultTarget();
	CodeGen::CodeGenerator codeGenerator(targetInfo, outputName);
	codeGenerator.genNamespace(rootSEMNamespace);
	codeGenerator.applyOptimisations(optLevel);
	codeGenerator.dumpToFile(outputName + ".ll");
	codeGenerator.writeToFile(outputName + ".bc");
	
	return 0;
}

