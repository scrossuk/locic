#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IRReader/IRReader.h>

#ifdef LLVM_3_5
#include <llvm/Linker/Linker.h>
#else
#include <llvm/Linker.h>
#endif

#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SourceMgr.h>

#include <locic/AST.hpp>
#include <locic/Debug.hpp>

#include <locic/Parser/DefaultParser.hpp>
#include <locic/CodeGen/CodeGen.hpp>
#include <locic/CodeGen/Interpreter.hpp>
#include <locic/CodeGen/Module.hpp>
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

// Force dependency on runtime ABI.
FORCE_UNDEFINED_SYMBOL(__loci_assert_failed)
FORCE_UNDEFINED_SYMBOL(__loci_allocate_exception)
FORCE_UNDEFINED_SYMBOL(__loci_free_exception)
FORCE_UNDEFINED_SYMBOL(__loci_throw)
FORCE_UNDEFINED_SYMBOL(__loci_get_exception)
FORCE_UNDEFINED_SYMBOL(__loci_personality_v0)

std::string loadFile(const std::string& fileName) {
	std::ifstream fileStream(fileName.c_str());
	std::stringstream buffer;
	buffer << fileStream.rdbuf();
	return buffer.str();
}

bool checkError(const std::string& testError, const std::string& expectedError) {
	auto testErrorCopy = testError;
	boost::algorithm::trim(testErrorCopy);
	
	auto expectedErrorCopy = expectedError;
	boost::algorithm::trim(expectedErrorCopy);
	boost::regex expectedErrorRegex(expectedErrorCopy);
	
	return boost::regex_match(testErrorCopy, expectedErrorRegex);
}

llvm::Module* loadBitcodeFile(const std::string& fileName, llvm::LLVMContext& context) {
#ifdef LLVM_3_5
	llvm::SMDiagnostic error;
	return llvm::ParseIRFile(fileName, error, context);
#else
	llvm::sys::Path fileNamePath;
	if (!fileNamePath.set(fileName)) {
		printf("Invalid file name '%s'.\n", fileName.c_str());
		return nullptr;
	}
	
	llvm::SMDiagnostic error;
	return llvm::ParseIRFile(fileNamePath.str(), error, context);
#endif
}

int main(int argc, char* argv[]) {
	if (argc < 1) {
		return -1;
	}
	
	const auto programName = boost::filesystem::path(argv[0]).stem().string();
	
	std::string testName;
	std::vector<std::string> inputFileNames;
	std::string entryPointName;
	std::string expectedErrorFileName;
	std::string expectedOutputFileName;
	int expectedResult = 0;
	std::vector<std::string> dependencyModules;
	std::vector<std::string> programArgs;
	
	po::options_description visibleOptions("Options");
	visibleOptions.add_options()
	("help,h", "Display help information")
	("test-name", po::value<std::string>(&testName), "Set test name")
	("entry-point", po::value<std::string>(&entryPointName)->default_value("testEntryPoint"), "Set entry point function name")
	("expected-error", po::value<std::string>(&expectedErrorFileName), "Set expected error file name")
	("expected-output", po::value<std::string>(&expectedOutputFileName), "Set expected output file name")
	("expected-result", po::value<int>(&expectedResult)->default_value(0), "Set expected result")
	("dependency-modules", po::value<std::vector<std::string>>(&dependencyModules)->multitoken(), "Set dependency module bitcode files")
	("args", po::value<std::vector<std::string>>(&programArgs)->multitoken(), "Set program arguments")
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
	
	if (expectedErrorFileName.empty() && expectedOutputFileName.empty()) {
		printf("%s: Must specify either an error filename or expected output filename.\n", programName.c_str());
		printf("Usage: %s [options] file...\n", programName.c_str());
		std::cout << visibleOptions << std::endl;
		return -1;
	}
	
	if (!expectedErrorFileName.empty() && !expectedOutputFileName.empty()) {
		printf("%s: Cannot specify both an error filename and expected output filename.\n", programName.c_str());
		printf("Usage: %s [options] file...\n", programName.c_str());
		std::cout << visibleOptions << std::endl;
		return -1;
	}
	
	inputFileNames.push_back("BuiltInTypes.loci");
	
	try {
		AST::NamespaceList astRootNamespaceList;
		
		std::stringstream parseErrors;
		
		// Parse all source files.
		for (const auto& filename: inputFileNames) {
			FILE* file = (filename == "BuiltInTypes.loci") ? builtInTypesFile() : fopen(filename.c_str(), "rb");
			
			if (file == NULL) {
				printf("Test FAILED: Failed to open file '%s'.\n", filename.c_str());
				return -1;
			}
			
			Parser::DefaultParser parser(astRootNamespaceList, file, filename);
			
			if (!parser.parseFile()) {
				const auto errors = parser.getErrors();
				assert(!errors.empty());
				
				parseErrors << "Parser Error: Failed to parse file '" << filename << "' with " << errors.size() << " errors:\n";
				
				for (const auto& error : errors) {
					parseErrors << "Parser Error (at " << error.location.toString() << "): " << error.message << "\n";
				}
			}
		}
		
		const auto parseErrorString = parseErrors.str();
		if (!parseErrorString.empty()) {
			if (!expectedErrorFileName.empty()) {
				const auto expectedError = loadFile(expectedErrorFileName);
				if (checkError(parseErrorString, expectedError)) {
					printf("Test PASSED (with expected error).\n");
					printf("Error:\n%s\n", parseErrorString.c_str());
					return 0;
				} else {
					printf("Test FAILED: Actual error doesn't match expected error.\n");
					printf("---Expected error:\n%s\n", expectedError.c_str());
					printf("---Actual error:\n%s\n", parseErrorString.c_str());
					return -1;
				}
			} else {
				printf("Test FAILED: Compilation failed unexpectedly.\n");
				printf("Error:\n%s\n", parseErrorString.c_str());
				return -1;
			}
		}
		
		// Build options.
		BuildOptions buildOptions;
		buildOptions.unsafe = false;
		
		// Debug information.
		Debug::Module debugModule;
		
		// Perform semantic analysis.
		printf("Performing semantic analysis...\n");
		SEM::Context semContext;
		SemanticAnalysis::Run(astRootNamespaceList, semContext, debugModule);
		
		// Dump SEM tree information.
		const auto semDebugFileName = testName + "_semdebug.txt";
		std::ofstream ofs(semDebugFileName.c_str(), std::ios_base::binary);
		ofs << formatMessage(semContext.rootNamespace()->toString());
		
		// Perform code generation.
		printf("Performing code generation...\n");
		
		CodeGen::CodeGenerator codeGenerator("test", debugModule, buildOptions);
		
		codeGenerator.genNamespace(semContext.rootNamespace());
		
		// Dump LLVM IR.
		const auto codeGenDebugFileName = testName + "_codegendebug.ll";
		codeGenerator.dumpToFile(codeGenDebugFileName);
		
		if (!expectedErrorFileName.empty()) {
			printf("Test FAILED: Program compiled successfully when it was expected to fail.\n");
			return -1;
		}
		
		llvm::Module* linkedModule = codeGenerator.module().getLLVMModulePtr();
		
		for (const auto& dependencyModuleName: dependencyModules) {
			const auto dependencyModule = loadBitcodeFile(dependencyModuleName, linkedModule->getContext());
			if (dependencyModule == nullptr) {
				printf("Test FAILED: Failed to load dependency module '%s'.\n",
					dependencyModuleName.c_str());
				return -1;
			}
			
			std::string errorMessage;
			const bool linkFailed = llvm::Linker::LinkModules(linkedModule, dependencyModule, llvm::Linker::DestroySource, &errorMessage);
			if (linkFailed) {
				printf("Test FAILED: Couldn't link with dependency module '%s'; error given was '%s'.\n",
					dependencyModuleName.c_str(), errorMessage.c_str());
				return -1;
			}
		}
		
		// Interpret the code.
		CodeGen::Interpreter interpreter(linkedModule);
		
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
		if (!expectedErrorFileName.empty()) {
			const auto testError = e.toString();
			const auto expectedError = loadFile(expectedErrorFileName);
			
			if (checkError(testError, expectedError)) {
				printf("Test PASSED.\n\n");
				printf("Error:\n%s\n", testError.c_str());
				return 0;
			} else {
				printf("Test FAILED: Actual error doesn't match expected error.\n");
				printf("---Expected error:\n%s\n", expectedError.c_str());
				printf("---Actual error:\n%s\n", testError.c_str());
				return -1;
			}
		} else {
			printf("Test FAILED: Compilation failed unexpectedly (errors should be shown above).\n");
			return -1;
		}
	}
}

