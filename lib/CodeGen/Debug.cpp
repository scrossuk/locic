#include <assert.h>

#include <string>

#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/Name.hpp>

namespace locic {

	namespace CodeGen {
	
		DebugBuilder::DebugBuilder(Module& module)
			: module_(module), builder_(module_.getLLVMModule()) { }
		
		DebugBuilder::~DebugBuilder() { }
		
		void DebugBuilder::finalize() {
			builder_.finalize();
		}
		
		llvm::DICompileUnit DebugBuilder::createCompileUnit(const DebugCompileUnit& compileUnitInfo) {
			// Pretend to be C++ for now.
			const unsigned language = llvm::dwarf::DW_LANG_C_plus_plus;
			const bool isOptimized = false;
			const unsigned runtimeVersion = 0;
			builder_.createCompileUnit(language, compileUnitInfo.fileName, compileUnitInfo.directoryName,
				compileUnitInfo.compilerName, isOptimized, compileUnitInfo.flags, runtimeVersion);
			return compileUnit();
		}
		
		llvm::DICompileUnit DebugBuilder::compileUnit() const {
			const auto namedNode = module_.getLLVMModule().getNamedMetadata("llvm.dbg.cu");
			assert(namedNode != nullptr);
			const auto node = namedNode->getOperand(0);
			assert(node != nullptr);
			return llvm::DICompileUnit(node);
		}
		
		llvm::DIFile DebugBuilder::createFile(const std::string& fileName, const std::string& directory) {
			return builder_.createFile(fileName, directory);
		}
		
		llvm::DISubprogram DebugBuilder::convertFunction(llvm::DIFile file, unsigned int lineNumber, bool isDefinition, const Name& name, llvm::Function* function) {
			const auto functionType = builder_.createSubroutineType(file, builder_.getOrCreateArray(std::vector<llvm::Value*>{}));
			const bool isLocalToUnit = false;
			const auto scopeLine = lineNumber;
			const auto flags = 0;
			const bool isOptimised = false;
			
			return builder_.createFunction(compileUnit(), name.toString(), mangleFunctionName(module_, name),
				file, lineNumber, functionType, isLocalToUnit, isDefinition, scopeLine,
				flags, isOptimised, function);
		}
		
		llvm::DIVariable DebugBuilder::convertVar(SEM::Var* var, bool isParam) {
			(void) var;
			(void) isParam;
			// TODO!
			return llvm::DIVariable();
		}
		
	}
	
}

