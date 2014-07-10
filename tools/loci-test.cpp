#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <locic/AST.hpp>
#include <locic/Debug.hpp>

#include <locic/Parser/DefaultParser.hpp>
#include <locic/CodeGen/CodeGen.hpp>
#include <locic/CodeGen/Interpreter.hpp>
#include <locic/SemanticAnalysis.hpp>

using namespace locic;

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
	va_list varArgList;
	
	size_t bufferSize = 1024;
	char stackBuffer[1024];
	std::vector<char> dynamicBuffer;
	char* buffer = &stackBuffer[0];
	
	while (true) {
		va_start(varArgList, format);
		const int needed = vsnprintf(buffer, bufferSize, format, varArgList);
		va_end(varArgList);
		
		// In case the buffer provided is too small, some
		// platforms return the needed buffer size, whereas
		// some simply return -1.
		if (needed <= (int)bufferSize && needed >= 0) {
			testOutput += std::string(buffer, (size_t) needed);
			testOutput += "\n";
			printf("%s\n", std::string(buffer, (size_t) needed).c_str());
			return;
		}
		
		// Need to increase buffer size; use needed size if available.
		bufferSize = (needed > 0) ? (needed + 1) : (bufferSize * 2);
		dynamicBuffer.resize(bufferSize);
		buffer = &dynamicBuffer[0];
	}
}

#if defined(_WIN32)
# if defined(_WIN64)
#  define FORCE_UNDEFINED_SYMBOL(x) __pragma(comment (linker, "/export:" #x))
# else
#  define FORCE_UNDEFINED_SYMBOL(x) __pragma(comment (linker, "/export:_" #x))
# endif
#else
# define FORCE_UNDEFINED_SYMBOL(x) extern "C" void x(void); void (*__ ## x ## _fp)(void)=&x;
#endif

// Force dependency on exception runtime ABI.
FORCE_UNDEFINED_SYMBOL(__loci_allocate_exception)
FORCE_UNDEFINED_SYMBOL(__loci_free_exception)
FORCE_UNDEFINED_SYMBOL(__loci_throw)
FORCE_UNDEFINED_SYMBOL(__loci_get_exception)
FORCE_UNDEFINED_SYMBOL(__loci_personality_v0)

int main(int argc, char* argv[]) {
	try {
		if (argc < 1) return -1;
		const auto programName = boost::filesystem::path(argv[0]).stem().string();
		
		std::string testName;
		std::vector<std::string> inputFileNames;
		std::string entryPointName;
		std::string expectedOutputFileName;
		int expectedResult = 0;
		std::vector<std::string> programArgs;
		
		po::options_description visibleOptions("Options");
		visibleOptions.add_options()
		("help,h", "Display help information")
		("test-name", po::value<std::string>(&testName), "Set test name")
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
		
		if (testName.empty()) {
			printf("%s: No test name specified.\n", programName.c_str());
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
		
		// Debug information.
		Debug::Module debugModule;
		
		// Perform semantic analysis.
		SEM::Context semContext;
		SemanticAnalysis::Run(astRootNamespaceList, semContext, debugModule);
		
		// Dump SEM tree information.
		const auto semDebugFileName = testName + "_semdebug.txt";
		std::ofstream ofs(semDebugFileName.c_str(), std::ios_base::binary);
		ofs << formatMessage(semContext.rootNamespace()->toString());
		
		// Perform code generation.
		CodeGen::TargetInfo targetInfo = CodeGen::TargetInfo::DefaultTarget();
		CodeGen::CodeGenerator codeGenerator(targetInfo, "test", debugModule);
		
		codeGenerator.genNamespace(semContext.rootNamespace());
		
		// Dump LLVM IR.
		const auto codeGenDebugFileName = testName + "_codegendebug.ll";
		codeGenerator.dumpToFile(codeGenDebugFileName);
		
		// Interpret the code.
		CodeGen::Interpreter interpreter(codeGenerator.module());
		
		// Treat entry point function as if it is 'main'.
		programArgs.insert(programArgs.begin(), "testProgram");
		
		const int result = interpreter.runAsMain(entryPointName, programArgs);
		
		if (result != expectedResult) {
			printf("Test FAILED: Result '%d' doesn't match expected result '%d'.\n",
				result, expectedResult);
			return -1;
		}
		
		std::ifstream expectedOutputFileStream(expectedOutputFileName.c_str());
		std::stringstream expectedOutputBuffer;
		expectedOutputBuffer << expectedOutputFileStream.rdbuf();
		
		const auto& expectedOutput = expectedOutputBuffer.str();
		
		if (testOutput != expectedOutput) {
			printf("Test FAILED: Actual output doesn't match expected output.\n");
			printf("---Expected output:\n%s\n", expectedOutput.c_str());
			printf("---Actual output:\n%s\n", testOutput.c_str());
			return -1;
		}
		
		printf("Test PASSED.\n\n");
		printf("Output:\n%s\n", testOutput.c_str());
		
		return 0;
	} catch (const Exception& e) {
		printf("Compilation failed (errors should be shown above).\n");
		return -1;
	}
}

