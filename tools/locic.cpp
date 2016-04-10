#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <chrono>
#include <fstream>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <locic/AST.hpp>
#include <locic/BuildOptions.hpp>
#include <locic/Debug.hpp>

#include <locic/Frontend/DiagnosticArray.hpp>
#include <locic/Frontend/DiagnosticRenderer.hpp>
#include <locic/Parser/DefaultParser.hpp>
#include <locic/CodeGen/CodeGenerator.hpp>
#include <locic/CodeGen/Context.hpp>
#include <locic/CodeGen/Interpreter.hpp>
#include <locic/CodeGen/ModulePtr.hpp>
#include <locic/CodeGen/TargetOptions.hpp>
#include <locic/SemanticAnalysis.hpp>
#include <locic/Support/SharedMaps.hpp>

using namespace locic;

namespace po = boost::program_options;

static std::string makeRepeatChar(char c, size_t numChars) {
	std::string s;
	for (size_t i = 0; i < numChars; i++) {
		s += c;
	}
	return s;
}

static std::string generateSpacedText(const std::string& text, size_t minSize) {
	const std::string BORDER = "====";
	const size_t BORDER_SIZE = BORDER.length();	
	const size_t MIN_SPACE_BORDER_SIZE = 1;
	
	const size_t spacedTextSize = text.length() + (BORDER_SIZE + MIN_SPACE_BORDER_SIZE) * 2;
	const size_t size = spacedTextSize < minSize ? minSize : spacedTextSize;
	const size_t spacingSize = size - text.length();
	const size_t numSpaces = spacingSize - BORDER_SIZE;
	if ((numSpaces % 2) == 0) {
		return BORDER + makeRepeatChar(' ', numSpaces / 2) + text + makeRepeatChar(' ', numSpaces / 2) + BORDER;
	} else {
		return BORDER + makeRepeatChar(' ', numSpaces / 2) + text + makeRepeatChar(' ', (numSpaces / 2) + 1) + BORDER;
	}
}

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

class Timer {
	public:
		Timer()
			: start_(std::chrono::steady_clock::now()) { }
			
		double getTime() const {
			const auto end = std::chrono::steady_clock::now();
			return double(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_).count()) / 1000000000.0;
		}
		
	private:
		std::chrono::steady_clock::time_point start_;
	
};

bool isStdOutATTY() {
	return isatty(fileno(stdout));
}

struct CompilerOptions {
	std::vector<std::string> inputFileNames;
	int optimisationLevel;
	std::string outputFileName;
	
	// Target
	std::string targetArchString;
	std::string targetCPUString;
	std::string targetFloatABIString;
	std::string targetFPUString;
	std::string targetTripleString;
	
	// Debug
	std::string astDebugFileName;
	std::string semDebugFileName;
	std::string codeGenDebugFileName;
	std::string optDebugFileName;
	
	// Interpreter
	std::string entryPointName;
	std::vector<std::string> programArgs;
	
	bool timingsEnabled;
	bool emitIRText;
	bool verifying;
	bool unsafe;
	bool interpret;
	
	CompilerOptions()
	: optimisationLevel(0), timingsEnabled(false), emitIRText(false),
	verifying(false), unsafe(false), interpret(false) { }
};

Optional<CompilerOptions> parseOptions(const int argc, char* argv[]) {
	if (argc < 1) {
		printf("Error: Argument count less than 1.\n");
		return None;
	}
	
	const auto programName = boost::filesystem::path(argv[0]).stem().string();
	
	CompilerOptions options;
	
	po::options_description visibleOptions("Options");
	visibleOptions.add_options()
	("help,h", "Display help information")
	("output-file,o", po::value<std::string>(&(options.outputFileName))->default_value("out.bc"), "Set output file name")
	("optimisation,O", po::value<int>(&(options.optimisationLevel))->default_value(0), "Set optimization level")
	("target", po::value<std::string>(&(options.targetTripleString)), "Target Triple")
	("march", po::value<std::string>(&(options.targetArchString)), "Target Architecture")
	("mcpu", po::value<std::string>(&(options.targetCPUString)), "Target CPU")
	("mfloat-abi", po::value<std::string>(&(options.targetFloatABIString)), "Target Float ABI")
	("mfpu", po::value<std::string>(&(options.targetFPUString)), "Target FPU")
	("unsafe", "Build in 'unsafe mode' (i.e. assert traps disabled, overflow traps disabled etc.)")
	("timings", "Print out timings of the compiler stages")
	("emit-llvm", "Emit LLVM IR text")
	("verify", "Verify code and accept failures")
	("ast-debug-file", po::value<std::string>(&(options.astDebugFileName)), "Set Parser AST tree debug output file")
	("sem-debug-file", po::value<std::string>(&(options.semDebugFileName)), "Set Semantic Analysis SEM tree debug output file")
	("codegen-debug-file", po::value<std::string>(&(options.codeGenDebugFileName)), "Set CodeGen LLVM IR debug output file")
	("opt-debug-file", po::value<std::string>(&(options.optDebugFileName)), "Set Optimiser LLVM IR debug output file")
	("interpret", "Interpret the given program")
	("entry-point", po::value<std::string>(&(options.entryPointName))->default_value("main"),
	 "Set entry point function name")
	("args", po::value<std::vector<std::string>>(&(options.programArgs))->multitoken(),
	 "Set program arguments")
	;
	
	po::options_description hiddenOptions;
	hiddenOptions.add_options()
	("input-file", po::value<std::vector<std::string>>(&(options.inputFileNames)), "Set input file names")
	;
	
	po::options_description allOptions;
	allOptions.add(visibleOptions).add(hiddenOptions);
	
	po::positional_options_description optionsPositions;
	optionsPositions.add("input-file", -1);
	
	po::variables_map variableMap;
	
	try {
		po::store(po::command_line_parser(argc, argv)
			.options(allOptions)
			.style(po::command_line_style::unix_style)
			.positional(optionsPositions).run(), variableMap);
		po::notify(variableMap);
	} catch (const po::error& e) {
		printf("%s: Command line parsing error: %s\n", programName.c_str(), e.what());
		printf("Usage: %s [options] file...\n", programName.c_str());
		return None;
	}
	
	if (!variableMap["help"].empty()) {
		printf("Usage: %s [options] file...\n", programName.c_str());
		std::cout << visibleOptions << std::endl;
		return None;
	}
	
	if (options.inputFileNames.empty()) {
		printf("%s: No files provided.\n", programName.c_str());
		printf("Usage: %s [options] file...\n", programName.c_str());
		std::cout << visibleOptions << std::endl;
		return None;
	}
	
	if (options.optimisationLevel < 0 || options.optimisationLevel > 3) {
		printf("%s: Invalid optimisation level '%d'.\n", programName.c_str(),
		       options.optimisationLevel);
		printf("Usage: %s [options] file...\n", programName.c_str());
		std::cout << visibleOptions << std::endl;
		return None;
	}
	
	options.timingsEnabled = !variableMap["timings"].empty();
	options.emitIRText = !variableMap["emit-llvm"].empty();
	options.verifying = !variableMap["verify"].empty();
	options.unsafe = !variableMap["unsafe"].empty();
	options.interpret = !variableMap["interpret"].empty();
	
	options.inputFileNames.push_back("BuiltInTypes.loci");
	
	return make_optional(std::move(options));
}

class Driver {
public:
	Driver(const CompilerOptions& options)
	: options_(options) { }
	
	void printDiagnostics() {
		if (diagArray_.diags().empty()) {
			return;
		}
		
		DiagnosticRenderer renderer(/*useColors=*/isStdOutATTY());
		
		for (const auto& diagPair: diagArray_.diags()) {
			renderer.emitDiagnostic(std::cout, *(diagPair.diag),
			                        diagPair.location);
		}
		
		renderer.emitDiagnosticSummary(std::cout);
	}
	
	bool runParser(AST::NamespaceList& astRootNamespaceList) {
		Timer timer;
		bool success = true;
		
		for (const auto& filename: options_.inputFileNames) {
			const auto file = (filename == "BuiltInTypes.loci") ? builtInTypesFile() : fopen(filename.c_str(), "rb");
			
			if (file == nullptr) {
				printf("Parser Error: Failed to open file '%s'.\n", filename.c_str());
				success = false;
				continue;
			}
			
			Parser::DefaultParser parser(sharedMaps_.stringHost(), astRootNamespaceList,
			                             file, filename, diagArray_);
			parser.parseFile();
		}
		
		if (options_.timingsEnabled) {
			printf("Parser: %f seconds.\n", timer.getTime());
		}
		
		if (!options_.astDebugFileName.empty()) {
			// If requested, dump AST information.
			dumpAST(astRootNamespaceList);
		}
		
		return success && !diagArray_.anyErrors();
	}
	
	void dumpAST(const AST::NamespaceList& astRootNamespaceList) {
		Timer timer;
		
		// If requested, dump AST tree information.
		std::ofstream ofs(options_.astDebugFileName.c_str(), std::ios_base::binary);
		
		for (size_t i = 0; i < astRootNamespaceList.size(); i++) {
			const std::string spacedFileName = generateSpacedText(options_.inputFileNames.at(i), 20);
			ofs << makeRepeatChar('=', spacedFileName.length()) << std::endl;
			ofs << spacedFileName << std::endl;
			ofs << makeRepeatChar('=', spacedFileName.length()) << std::endl;
			ofs << std::endl;
			ofs << formatMessage(astRootNamespaceList.at(i).toString());
			ofs << std::endl << std::endl;
		}
		
		if (options_.timingsEnabled) {
			printf("Dump AST: %f seconds.\n", timer.getTime());
		}
	}
	
	bool runSemanticAnalysis(const AST::NamespaceList& astRootNamespaceList,
	                         SEM::Module& semModule, Debug::Module& debugModule) {
		Timer timer;
		SemanticAnalysis::Run(sharedMaps_, astRootNamespaceList,
		                      semModule, debugModule, diagArray_);
		
		if (options_.timingsEnabled) {
			printf("Semantic Analysis: %f seconds.\n", timer.getTime());
		}
		
		if (!options_.semDebugFileName.empty()) {
			// If requested, dump SEM information.
			dumpSEM(semModule);
		}
		
		return !diagArray_.anyErrors();
	}
	
	void dumpSEM(const SEM::Module& semModule) {
		Timer timer;
		
		// If requested, dump SEM tree information.
		std::ofstream ofs(options_.semDebugFileName.c_str(), std::ios_base::binary);
		ofs << formatMessage(semModule.rootNamespace().toString());
		
		if (options_.timingsEnabled) {
			printf("Dump SEM: %f seconds.\n", timer.getTime());
		}
	}
	
	SharedMaps& sharedMaps() {
		return sharedMaps_;
	}
	
	CodeGen::TargetOptions getTargetOptions() const {
		CodeGen::TargetOptions targetOptions;
		targetOptions.triple = options_.targetTripleString;
		targetOptions.arch = options_.targetArchString;
		targetOptions.cpu = options_.targetCPUString;
		targetOptions.floatABI = options_.targetFloatABIString;
		targetOptions.fpu = options_.targetFPUString;
		return targetOptions;
	}
	
	CodeGen::ModulePtr
	runCodeGen(CodeGen::Context& codeGenContext, SEM::Module& semModule,
	           Debug::Module& debugModule) {
		BuildOptions buildOptions;
		buildOptions.unsafe = options_.unsafe;
		
		CodeGen::CodeGenerator codeGenerator(codeGenContext, options_.outputFileName,
		                                     debugModule, buildOptions);
		
		{
			Timer timer;
			codeGenerator.genNamespace(&(semModule.rootNamespace()));
			
			if (options_.timingsEnabled) {
				printf("Code Generation: %f seconds.\n", timer.getTime());
			}
		}
		
		if (!options_.codeGenDebugFileName.empty()) {
			Timer timer;
			
			// If requested, dump LLVM IR prior to optimisation.
			codeGenerator.dumpToFile(options_.codeGenDebugFileName);
			
			if (options_.timingsEnabled) {
				printf("Dump LLVM IR (pre optimisation): %f seconds.\n", timer.getTime());
			}
		}
		
		{
			Timer timer;
			codeGenerator.applyOptimisations(options_.optimisationLevel);
			if (options_.timingsEnabled) {
				printf("Optimisation: %f seconds.\n", timer.getTime());
			}
		}
		
		if (!options_.optDebugFileName.empty()) {
			Timer timer;
			
			// If requested, dump LLVM IR after optimisation.
			codeGenerator.dumpToFile(options_.optDebugFileName);
			
			if (options_.timingsEnabled) {
				printf("Dump LLVM IR (post optimisation): %f seconds.\n", timer.getTime());
			}
		}
		
		if (options_.interpret) {
			// If we're interpreting, don't output any code.
			return codeGenerator.releaseModule();
		}
		
		if (options_.emitIRText) {
			Timer timer;
			codeGenerator.dumpToFile(options_.outputFileName);
			if (options_.timingsEnabled) {
				printf("Write IR Assembly: %f seconds.\n", timer.getTime());
			}
		} else {
			Timer timer;
			codeGenerator.writeToFile(options_.outputFileName);
			if (options_.timingsEnabled) {
				printf("Write Bitcode: %f seconds.\n", timer.getTime());
			}
		}
		
		return codeGenerator.releaseModule();
	}
	
	int interpret(CodeGen::Context& codeGenContext, CodeGen::ModulePtr module) {
		CodeGen::Interpreter interpreter(codeGenContext, std::move(module));
		
		// Treat entry point function as if it is 'main' by passing in
		// a fake program name.
		auto programArgs = options_.programArgs;
		programArgs.insert(programArgs.begin(), "<interpreted>");
		
		return interpreter.runAsMain(options_.entryPointName,
		                             programArgs);
	}
	
private:
	const CompilerOptions& options_;
	DiagnosticArray diagArray_;
	SharedMaps sharedMaps_;
	
};

#if defined(_WIN32)
# if defined(_WIN64)
#  define FORCE_UNDEFINED_SYMBOL(x) __pragma(comment (linker, "/export:" #x))
# else
#  define FORCE_UNDEFINED_SYMBOL(x) __pragma(comment (linker, "/export:_" #x))
# endif
#else
# define FORCE_UNDEFINED_SYMBOL(x) extern "C" void x(void); void (*__ ## x ## _fp)(void)=&x;
#endif

// Force dependency on runtime ABI, for the benefit of interpreted code.
FORCE_UNDEFINED_SYMBOL(__loci_assert_failed)
FORCE_UNDEFINED_SYMBOL(__loci_allocate_exception)
FORCE_UNDEFINED_SYMBOL(__loci_free_exception)
FORCE_UNDEFINED_SYMBOL(__loci_throw)
FORCE_UNDEFINED_SYMBOL(__loci_get_exception)
FORCE_UNDEFINED_SYMBOL(__loci_personality_v0)

int main(int argc, char* argv[]) {
	Timer totalTimer;
	
	const auto options = parseOptions(argc, argv);
	if (!options) {
		return EXIT_FAILURE;
	}
	
	try {
		Driver driver(*options);
		
		AST::NamespaceList astRootNamespaceList;
		
		const bool parseResult = driver.runParser(astRootNamespaceList);
		if (!parseResult) {
			driver.printDiagnostics();
			return options->verifying ? EXIT_SUCCESS : EXIT_FAILURE;
		}
		
		// Debug information.
		Debug::Module debugModule;
		
		SEM::Context semContext;
		SEM::Module semModule(semContext);
		
		const auto semaResult = driver.runSemanticAnalysis(astRootNamespaceList,
		                                                   semModule, debugModule);
		driver.printDiagnostics();
		if (!semaResult) {
			return options->verifying ? EXIT_SUCCESS : EXIT_FAILURE;
		}
		
		if (options->verifying) {
			return EXIT_SUCCESS;
		}
		
		CodeGen::Context codeGenContext(semContext, driver.sharedMaps(),
		                                driver.getTargetOptions());
		
		auto irModule = driver.runCodeGen(codeGenContext, semModule,
		                                  debugModule);
		
		if (options->timingsEnabled) {
			printf("--- Total time: %f seconds.\n", totalTimer.getTime());
		}
		
		if (!options->interpret) {
			return EXIT_SUCCESS;
		}
		
		return driver.interpret(codeGenContext, std::move(irModule));
	} catch (const Exception& e) {
		printf("Compilation failed (errors should be shown above).\n");
		return options->verifying ? EXIT_SUCCESS : EXIT_FAILURE;
	}
	
	return 0;
}

