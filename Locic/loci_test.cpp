#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <Locic/AST.hpp>
#include <Locic/Parser/DefaultParser.hpp>
#include <Locic/CodeGen/CodeGen.hpp>
#include <Locic/SemanticAnalysis.hpp>

using namespace Locic;

namespace po = boost::program_options;

extern unsigned char BuiltInTypes_loci[];
extern unsigned int BuiltInTypes_loci_len;

static FILE* builtInTypesFile() {
	FILE* file = tmpfile();
	
	const size_t writeSize = fwrite((const void*) BuiltInTypes_loci, sizeof(unsigned char), (size_t) BuiltInTypes_loci_len, file);
	if (writeSize != (size_t) BuiltInTypes_loci_len) {
		fclose(file);
		return NULL;
	}
	
	rewind(file);
	
	return file;
}

std::string testOutput;

// This function will be called by the Loci
// code being tested.
extern "C" void testPrint(const char* format, ...) {
	// TODO...
	testOutput += "testPrint";
}

int main(int argc, char* argv[]) {
	try {
		if (argc < 1) return -1;
		const std::string programName = boost::filesystem::path(argv[0]).stem().string();
		
		std::vector<std::string> inputFileNames;
		std::string entryPointName;
		std::string expectedOutputFileName;
		int expectedResult = 0;
		std::vector<std::string> programArgs;
		
		po::options_description visibleOptions("Options");
		visibleOptions.add_options()
		("help,h", "Display help information")
		("entry-point", po::value<std::string>(&entryPointName)->default_value("testEntryPoint"), "Set entry point function name")
		("expected-output", po::value<std::string>(&expectedOutputFileName), "Set expected output file name")
		("expected-result", po::value<int>(&expectedResult)->default_value(0), "Set expected result")
		("args", po::value<std::vector<std::string>>(&programArgs), "Set program arguments")
		;
		
		po::options_description hiddenOptions;
		hiddenOptions.add_options()
		("input-file", po::value<std::vector<std::string>>(&inputFileNames), "Set input file names")
		;
		
		po::options_description allOptions;
		allOptions.add(visibleOptions).add(hiddenOptions);
		
		po::positional_options_description optionsPositions;
		optionsPositions.add("input-file", -1);
		
		po::variables_map variableMap;
		
		try {
			po::store(po::command_line_parser(argc, argv).options(allOptions).positional(optionsPositions).run(), variableMap);
			po::notify(variableMap);
		} catch (const po::error& e) {
			printf("%s: Command line parsing error: %s\n", programName.c_str(), e.what());
			printf("Usage: %s [options] file...\n", programName.c_str());
			std::cout << visibleOptions << std::endl;
			return -1;
		}
		
		if (!variableMap["help"].empty()) {
			printf("Usage: %s [options] file...\n", programName.c_str());
			std::cout << visibleOptions << std::endl;
			return -1;
		}
		
		if (inputFileNames.empty()) {
			printf("%s: No files provided.\n", programName.c_str());
			printf("Usage: %s [options] file...\n", programName.c_str());
			std::cout << visibleOptions << std::endl;
			return -1;
		}
		
		if (expectedOutputFileName.empty()) {
			printf("%s: No expected output filename specified.\n", programName.c_str());
			printf("Usage: %s [options] file...\n", programName.c_str());
			std::cout << visibleOptions << std::endl;
			return -1;
		}
		
		inputFileNames.push_back("BuiltInTypes.loci");
		
		AST::NamespaceList astRootNamespaceList;
		
		// Parse all source files.
		for (std::size_t i = 0; i < inputFileNames.size(); i++) {
			const std::string filename = inputFileNames.at(i);
			FILE* file = (filename == "BuiltInTypes.loci") ? builtInTypesFile() : fopen(filename.c_str(), "rb");
			
			if (file == NULL) {
				printf("Parser Error: Failed to open file '%s'.\n", filename.c_str());
				return -1;
			}
			
			Parser::DefaultParser parser(astRootNamespaceList, file, filename);
			
			if (!parser.parseFile()) {
				const auto errors = parser.getErrors();
				assert(!errors.empty());
				
				printf("Parser Error: Failed to parse file '%s' with %lu errors:\n", filename.c_str(), errors.size());
				
				for (const auto & error : errors) {
					printf("Parser Error (at %s): %s\n", error.location.toString().c_str(), error.message.c_str());
				}
				
				return -1;
			}
		}
		
		// Perform semantic analysis.
		SEM::Namespace* rootSEMNamespace = SemanticAnalysis::Run(astRootNamespaceList);
		assert(rootSEMNamespace != NULL);
		
		// Perform code generation.
		CodeGen::TargetInfo targetInfo = CodeGen::TargetInfo::DefaultTarget();
		CodeGen::CodeGenerator codeGenerator(targetInfo, "test");
		codeGenerator.genNamespace(rootSEMNamespace);
		
		// TODO...
		return -1;
		
		// Interpret the code.
		/*CodeGen::Interpreter interpreter;
		interpreter.addCodeGenerator(codeGenerator);
		
		// Treat entry point function as if it is 'main'.
		const int result = interpreter.run(entryPointName, programArgs);
		
		if (result != expectedResult) {
			printf("Test FAILED: Result '%d' doesn't match expected result '%d'.\n",
				result, expectedResult);
			return -1;
		}
		
		// TODO...
		
		printf("Test PASSED.\n");*/
		
	} catch (const Exception& e) {
		printf("Compilation failed (errors should be shown above).\n");
		return -1;
	}
	
	return 0;
}

