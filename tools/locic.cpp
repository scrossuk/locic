#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <chrono>
#include <fstream>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <locic/AST.hpp>
#include <locic/BuildOptions.hpp>
#include <locic/Debug.hpp>

#include <locic/Parser/DefaultParser.hpp>
#include <locic/CodeGen/CodeGenerator.hpp>
#include <locic/CodeGen/Context.hpp>
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

int main(int argc, char* argv[]) {
	Timer totalTimer;
	
	if (argc < 1) return -1;
	const auto programName = boost::filesystem::path(argv[0]).stem().string();
	
	std::vector<std::string> inputFileNames;
	int optimisationLevel = 0;
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
	
	po::options_description visibleOptions("Options");
	visibleOptions.add_options()
	("help,h", "Display help information")
	("output-file,o", po::value<std::string>(&outputFileName)->default_value("out.bc"), "Set output file name")
	("optimisation,O", po::value<int>(&optimisationLevel)->default_value(0), "Set optimization level")
	("target", po::value<std::string>(&targetTripleString), "Target Triple")
	("march", po::value<std::string>(&targetArchString), "Target Architecture")
	("mcpu", po::value<std::string>(&targetCPUString), "Target CPU")
	("mfloat-abi", po::value<std::string>(&targetFloatABIString), "Target Float ABI")
	("mfpu", po::value<std::string>(&targetFPUString), "Target FPU")
	("unsafe", "Build in 'unsafe mode' (i.e. assert traps disabled, overflow traps disabled etc.)")
	("timings", "Print out timings of the compiler stages")
	("emit-llvm", "Emit LLVM IR text")
	("verify", "Verify code and accept failures")
	("ast-debug-file", po::value<std::string>(&astDebugFileName), "Set Parser AST tree debug output file")
	("sem-debug-file", po::value<std::string>(&semDebugFileName), "Set Semantic Analysis SEM tree debug output file")
	("codegen-debug-file", po::value<std::string>(&codeGenDebugFileName), "Set CodeGen LLVM IR debug output file")
	("opt-debug-file", po::value<std::string>(&optDebugFileName), "Set Optimiser LLVM IR debug output file")
	("use-new-parser", "Use new (handwritten) parser")
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
		po::store(po::command_line_parser(argc, argv)
			.options(allOptions)
			.style(po::command_line_style::unix_style)
			.positional(optionsPositions).run(), variableMap);
		po::notify(variableMap);
	} catch (const po::error& e) {
		printf("%s: Command line parsing error: %s\n", programName.c_str(), e.what());
		printf("Usage: %s [options] file...\n", programName.c_str());
		return 1;
	}
	
	if (!variableMap["help"].empty()) {
		printf("Usage: %s [options] file...\n", programName.c_str());
		std::cout << visibleOptions << std::endl;
		return 1;
	}
	
	if (inputFileNames.empty()) {
		printf("%s: No files provided.\n", programName.c_str());
		printf("Usage: %s [options] file...\n", programName.c_str());
		std::cout << visibleOptions << std::endl;
		return 1;
	}
	
	if (optimisationLevel < 0 || optimisationLevel > 3) {
		printf("%s: Invalid optimisation level '%d'.\n", programName.c_str(), optimisationLevel);
		printf("Usage: %s [options] file...\n", programName.c_str());
		std::cout << visibleOptions << std::endl;
		return 1;
	}
	
	const bool timingsEnabled = !variableMap["timings"].empty();
	const bool emitIRText = !variableMap["emit-llvm"].empty();
	const bool verifying = !variableMap["verify"].empty();
	const bool useNewParser = !variableMap["use-new-parser"].empty();
	
	BuildOptions buildOptions;
	buildOptions.unsafe = !variableMap["unsafe"].empty();
	
	inputFileNames.push_back("BuiltInTypes.loci");
	
	try {
		SharedMaps sharedMaps;
		
		AST::NamespaceList astRootNamespaceList;
		
		{
			Timer timer;
			
			// Parse all source files.
			for (const auto& filename: inputFileNames) {
				const auto file = (filename == "BuiltInTypes.loci") ? builtInTypesFile() : fopen(filename.c_str(), "rb");
				
				if (file == nullptr) {
					printf("Parser Error: Failed to open file '%s'.\n", filename.c_str());
					return 1;
				}
				
				Parser::DefaultParser parser(sharedMaps.stringHost(), astRootNamespaceList, file, filename, useNewParser);
				
				if (!parser.parseFile()) {
					const auto errors = parser.getErrors();
					assert(!errors.empty());
					
					printf("Parser Error: Failed to parse file '%s' with %lu errors:\n", filename.c_str(), errors.size());
					
					for (const auto & error : errors) {
						printf("Parser Error (at %s): %s\n", error.location.toString().c_str(), error.message.c_str());
					}
					
					return 1;
				}
			}
			
			if (timingsEnabled) {
				printf("Parser: %f seconds.\n", timer.getTime());
			}
		}
		
		if (!astDebugFileName.empty()) {
			Timer timer;
			
			// If requested, dump AST tree information.
			std::ofstream ofs(astDebugFileName.c_str(), std::ios_base::binary);
			
			for (size_t i = 0; i < astRootNamespaceList.size(); i++) {
				const std::string spacedFileName = generateSpacedText(inputFileNames.at(i), 20);
				ofs << makeRepeatChar('=', spacedFileName.length()) << std::endl;
				ofs << spacedFileName << std::endl;
				ofs << makeRepeatChar('=', spacedFileName.length()) << std::endl;
				ofs << std::endl;
				ofs << formatMessage(astRootNamespaceList.at(i).toString());
				ofs << std::endl << std::endl;
			}
			
			if (timingsEnabled) {
				printf("Dump AST: %f seconds.\n", timer.getTime());
			}
		}
		
		// Debug information.
		Debug::Module debugModule;
		
		SEM::Context semContext;
		SEM::Module semModule(semContext);
		
		// Perform semantic analysis.
		{
			Timer timer;
			SemanticAnalysis::Run(sharedMaps, astRootNamespaceList,
			                      semModule, debugModule);
			
			if (timingsEnabled) {
				printf("Semantic Analysis: %f seconds.\n", timer.getTime());
			}
		}
		
		if (!semDebugFileName.empty()) {
			Timer timer;
			
			// If requested, dump SEM tree information.
			std::ofstream ofs(semDebugFileName.c_str(), std::ios_base::binary);
			ofs << formatMessage(semModule.rootNamespace().toString());
			
			if (timingsEnabled) {
				printf("Dump SEM: %f seconds.\n", timer.getTime());
			}
		}
		
		if (verifying) {
			return 0;
		}
		
		CodeGen::TargetOptions targetOptions;
		targetOptions.triple = targetTripleString;
		targetOptions.arch = targetArchString;
		targetOptions.cpu = targetCPUString;
		targetOptions.floatABI = targetFloatABIString;
		targetOptions.fpu = targetFPUString;
		
		CodeGen::Context codeGenContext(semContext, sharedMaps, targetOptions);
		CodeGen::CodeGenerator codeGenerator(codeGenContext, outputFileName, debugModule, buildOptions);
		
		{
			Timer timer;
			codeGenerator.genNamespace(&(semModule.rootNamespace()));
			
			if (timingsEnabled) {
				printf("Code Generation: %f seconds.\n", timer.getTime());
			}
		}
		
		if (!codeGenDebugFileName.empty()) {
			Timer timer;
			
			// If requested, dump LLVM IR prior to optimisation.
			codeGenerator.dumpToFile(codeGenDebugFileName);
			
			if (timingsEnabled) {
				printf("Dump LLVM IR (pre optimisation): %f seconds.\n", timer.getTime());
			}
		}
		
		{
			Timer timer;
			codeGenerator.applyOptimisations(optimisationLevel);
			if (timingsEnabled) {
				printf("Optimisation: %f seconds.\n", timer.getTime());
			}
		}
		
		if (!optDebugFileName.empty()) {
			Timer timer;
			
			// If requested, dump LLVM IR after optimisation.
			codeGenerator.dumpToFile(optDebugFileName);
			
			if (timingsEnabled) {
				printf("Dump LLVM IR (post optimisation): %f seconds.\n", timer.getTime());
			}
		}
		
		if (emitIRText) {
			Timer timer;
			codeGenerator.dumpToFile(outputFileName);
			if (timingsEnabled) {
				printf("Write IR Assembly: %f seconds.\n", timer.getTime());
			}
		} else {
			Timer timer;
			codeGenerator.writeToFile(outputFileName);
			if (timingsEnabled) {
				printf("Write Bitcode: %f seconds.\n", timer.getTime());
			}
		}
		
		if (timingsEnabled) {
			printf("--- Total time: %f seconds.\n", totalTimer.getTime());
		}
	} catch (const Exception& e) {
		printf("Compilation failed (errors should be shown above).\n");
		return verifying ? 0 : 1;
	}
	
	return 0;
}

