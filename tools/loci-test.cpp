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

#include <locic/AST.hpp>
#include <locic/BuildOptions.hpp>
#include <locic/Debug.hpp>

#include <locic/Parser/DefaultParser.hpp>
#include <locic/CodeGen/Context.hpp>
#include <locic/CodeGen/CodeGenerator.hpp>
#include <locic/CodeGen/Interpreter.hpp>
#include <locic/CodeGen/Linker.hpp>
#include <locic/CodeGen/ModulePtr.hpp>
#include <locic/CodeGen/TargetOptions.hpp>
#include <locic/SemanticAnalysis.hpp>
#include <locic/Support/SharedMaps.hpp>

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

// This function will be called by the Loci code being tested.
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

class TestFailedException: public std::exception { };

class TestFailedToOpenFileException: public TestFailedException {
public:
	TestFailedToOpenFileException(const std::string& filename)
	: error_(makeString("Failed to open file '%s'.", filename.c_str())) { }
	
	const char* what() const noexcept {
		return error_.c_str();
	}
	
private:
	std::string error_;
	
};

class TestUnexpectedErrorException: public TestFailedException {
public:
	TestUnexpectedErrorException(const std::string& actual,
	                             const std::string& expected)
	: error_(makeString("Actual error doesn't match expected error.\n"
	                    "---Expected error:\n%s\n"
	                    "---Actual error:\n%s\n",
	                    expected.c_str(),
	                    actual.c_str())) { }
	
	const char* what() const noexcept {
		return error_.c_str();
	}
	
private:
	std::string error_;
	
};

class TestUnexpectedFailureException: public TestFailedException {
public:
	TestUnexpectedFailureException(const std::string& error)
	: error_(makeString("Compilation failed unexpectedly.\n"
	                    "---Error:\n%s\n",
	                    error.c_str())) { }
	
	const char* what() const noexcept {
		return error_.c_str();
	}
	
private:
	std::string error_;
	
};

class TestUnexpectedOutputException: public TestFailedException {
public:
	TestUnexpectedOutputException(const std::string& actual,
	                              const std::string& expected)
	: error_(makeString("Actual output doesn't match expected output.\n"
	                    "---Expected output:\n%s\n"
	                    "---Actual output:\n%s\n",
	                    expected.c_str(),
	                    actual.c_str())) { }
	
	const char* what() const noexcept {
		return error_.c_str();
	}
	
private:
	std::string error_;
	
};

class TestUnexpectedResultException: public TestFailedException {
public:
	TestUnexpectedResultException(const int actual,
	                              const int expected)
	: error_(makeString("Actual result doesn't match expected result.\n"
	                    "---Expected result: %d\n"
	                    "---Actual result: %d\n",
	                    expected, actual)) { }
	
	const char* what() const noexcept {
		return error_.c_str();
	}
	
private:
	std::string error_;
	
};

class TestUnexpectedSuccessException: public TestFailedException {
public:
	TestUnexpectedSuccessException()
	: error_("Program compiled successfully when it was expected to fail.") { }
	
	const char* what() const noexcept {
		return error_.c_str();
	}
	
private:
	std::string error_;
	
};

struct TestOptions {
	bool dumpOutput;
	bool isHelpRequested;
	std::string testName;
	std::vector<std::string> inputFileNames;
	std::string entryPointName;
	bool parseOnly;
	std::string expectedErrorFileName;
	std::string expectedOutputFileName;
	int expectedResult;
	std::vector<std::string> dependencyModules;
	std::vector<std::string> programArgs;
	
	TestOptions()
	: dumpOutput(false),
	isHelpRequested(false),
	parseOnly(false),
	expectedResult(0) { }
};

bool runTest(TestOptions& options) {
	try {
		AST::NamespaceList astRootNamespaceList;
		
		SharedMaps sharedMaps;
		
		std::stringstream parseErrors;
		
		// Parse all source files.
		for (const auto& filename: options.inputFileNames) {
			FILE* file = (filename == "BuiltInTypes.loci") ? builtInTypesFile() : fopen(filename.c_str(), "rb");
			
			if (file == NULL) {
				throw TestFailedToOpenFileException(filename);
			}
			
			Parser::DefaultParser parser(sharedMaps.stringHost(), astRootNamespaceList, file, filename);
			
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
			if (!options.expectedErrorFileName.empty()) {
				const auto expectedError = loadFile(options.expectedErrorFileName);
				if (checkError(parseErrorString, expectedError)) {
					printf("Test PASSED (with expected error).\n");
					printf("Error:\n%s\n", parseErrorString.c_str());
					return true;
				} else {
					throw TestUnexpectedErrorException(parseErrorString, expectedError);
				}
			} else {
				throw TestUnexpectedFailureException(parseErrorString);
			}
		}
		
		if (options.parseOnly) {
			if (!options.expectedErrorFileName.empty()) {
				throw TestUnexpectedSuccessException();
			}
			printf("Test PASSED (successfully parsed source file).\n");
			return true;
		}
		
		// Build options.
		BuildOptions buildOptions;
		buildOptions.unsafe = false;
		
		// Debug information.
		Debug::Module debugModule;
		
		// Perform semantic analysis.
		printf("Performing semantic analysis...\n");
		
		SEM::Context semContext;
		
		// TODO: Clean up this try-catch!
		try {
			SemanticAnalysis::Run(sharedMaps, astRootNamespaceList, semContext, debugModule);
		} catch (const Exception& e) {
			if (!options.expectedErrorFileName.empty()) {
				const auto testError = e.toString();
				const auto expectedError = loadFile(options.expectedErrorFileName);
				
				if (checkError(testError, expectedError)) {
					printf("Test PASSED.\n\n");
					printf("Error:\n%s\n", testError.c_str());
					return true;
				} else {
					throw TestUnexpectedErrorException(testError, expectedError);
				}
			} else {
				throw TestUnexpectedFailureException(e.toString());
			}
		}
		
		if (options.dumpOutput) {
			// Dump SEM tree information.
			const auto semDebugFileName = options.testName + "_semdebug.txt";
			std::ofstream ofs(semDebugFileName.c_str(), std::ios_base::binary);
			ofs << formatMessage(semContext.rootNamespace()->toString());
		}
		
		// Perform code generation.
		printf("Performing code generation...\n");
		
		// Use default (host) target options.
		CodeGen::Context codeGenContext(sharedMaps, CodeGen::TargetOptions());
		CodeGen::CodeGenerator codeGenerator(codeGenContext, "test", debugModule, buildOptions);
		
		codeGenerator.genNamespace(semContext.rootNamespace());
		
		if (options.dumpOutput) {
			// Dump LLVM IR.
			const auto codeGenDebugFileName = options.testName + "_codegendebug.ll";
			codeGenerator.dumpToFile(codeGenDebugFileName);
		}
		
		if (!options.expectedErrorFileName.empty()) {
			throw TestUnexpectedSuccessException();
		}
		
		if (options.expectedOutputFileName.empty()) {
			// No output file was specified, so we just needed to
			// check that the input files can be compiled.
			return true;
		}
		
		CodeGen::Linker linker(codeGenContext, codeGenerator.releaseModule());
		
		printf("Linking...\n");
		
		for (const auto& dependencyModuleName: options.dependencyModules) {
			linker.loadModule(dependencyModuleName);
		}
		
		printf("Starting interpreter...\n");
		
		// Interpret the code.
		CodeGen::Interpreter interpreter(codeGenContext, linker.releaseModule());
		
		// Treat entry point function as if it is 'main'.
		options.programArgs.insert(options.programArgs.begin(), "testProgram");
		
		printf("Running...\n");
		
		const int result = interpreter.runAsMain(options.entryPointName,
		                                         options.programArgs);
		
		if (result != options.expectedResult) {
			throw TestUnexpectedResultException(result,
			                                    options.expectedResult);
		}
		
		printf("Analysing output...\n");
		
		const auto expectedOutput = loadFile(options.expectedOutputFileName);
		
		if (testOutput != expectedOutput) {
			throw TestUnexpectedOutputException(testOutput,
			                                    expectedOutput);
		}
		
		printf("Test PASSED.\n\n");
		printf("Output:\n%s\n", testOutput.c_str());
		return true;
	} catch (const TestFailedException& e) {
		printf("Test FAILED: %s\n", e.what());
		return false;
	}
}

class CommandLineParser {
public:
	CommandLineParser()
	: visibleOptions_("Options") {
		visibleOptions_.add_options()
		("help,h", "Display help information")
		("test-name", po::value<std::string>(), "Set test name")
		("entry-point", po::value<std::string>()->default_value("testEntryPoint"), "Set entry point function name")
		("parse-only", "Only perform parsing stage")
		("expected-error", po::value<std::string>()->default_value(""), "Set expected error file name")
		("expected-output", po::value<std::string>()->default_value(""), "Set expected output file name")
		("expected-result", po::value<int>()->default_value(0), "Set expected result")
		("dependency-modules", po::value<std::vector<std::string>>()->multitoken(), "Set dependency module bitcode files")
		("args", po::value<std::vector<std::string>>()->multitoken(), "Set program arguments")
		;
		
		hiddenOptions_.add_options()
		("input-file", po::value<std::vector<std::string>>(), "Set input file names")
		;
		
		allOptions_.add(visibleOptions_).add(hiddenOptions_);
	}
	
	void printUsage() {
		printf("Usage: loci-test [options] file...\n");
		std::cout << visibleOptions_ << std::endl;
	}
	
	TestOptions parse(int argc, char* argv[]) {
		po::positional_options_description optionsPositions;
		optionsPositions.add("input-file", -1);
		
		po::variables_map variableMap;
		try {
			po::store(po::command_line_parser(argc, argv).options(allOptions_).positional(optionsPositions).run(), variableMap);
		} catch (const po::error& e) {
			throw std::runtime_error(makeString("Command line parsing error: %s", e.what()));
		}
		
		TestOptions options;
		options.isHelpRequested = !variableMap["help"].empty();
		
		options.testName = variableMap["test-name"].as<std::string>();
		if (options.testName.empty()) {
			throw std::runtime_error("No test name specified.");
		}
		
		options.entryPointName = variableMap["entry-point"].as<std::string>();
		options.parseOnly = !variableMap["parse-only"].empty();
		options.expectedErrorFileName = variableMap["expected-error"].as<std::string>();
		options.expectedOutputFileName = variableMap["expected-output"].as<std::string>();
		
		if (variableMap.count("dependency-modules") > 0) {
			options.dependencyModules = variableMap["dependency-modules"].as<std::vector<std::string>>();
		}
		
		if (variableMap.count("args") > 0) {
			options.programArgs = variableMap["args"].as<std::vector<std::string>>();
		}
		
		if (variableMap.count("input-file") > 0) {
			options.inputFileNames = variableMap["input-file"].as<std::vector<std::string>>();
		} else {
			throw std::runtime_error("No source files provided.");
		}
		
		if (!options.expectedErrorFileName.empty() &&
		    !options.expectedOutputFileName.empty()) {
			throw std::runtime_error("Cannot specify both an error filename and expected output filename.");
		}
		
		return options;
	}
	
private:
	po::options_description visibleOptions_;
	po::options_description hiddenOptions_;
	po::options_description allOptions_;
	
};

int main(int argc, char* argv[]) {
	if (argc < 1) {
		return EXIT_FAILURE;
	}
	
	CommandLineParser commandLineParser;
	
	try {
		TestOptions options = commandLineParser.parse(argc, argv);
		options.inputFileNames.push_back("BuiltInTypes.loci");
		
		if (options.isHelpRequested) {
			commandLineParser.printUsage();
			return EXIT_SUCCESS;
		}
		
		return runTest(options) ? EXIT_SUCCESS : EXIT_FAILURE;
	} catch (const std::exception& e) {
		printf("ERROR: %s\n", e.what());
		commandLineParser.printUsage();
		return EXIT_FAILURE;
	}
}

