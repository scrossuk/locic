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
		
		void DebugBuilder::createCompileUnit(const DebugCompileUnit& compileUnit) {
			// Pretend to be C++ for now.
			const unsigned language = llvm::dwarf::DW_LANG_C_plus_plus;
			const bool isOptimized = false;
			const unsigned runtimeVersion = 1;
			builder_.createCompileUnit(language, compileUnit.fileName, compileUnit.directoryName,
				compileUnit.compilerName, isOptimized, compileUnit.flags, runtimeVersion);
		}
		
		/*llvm::DICompileUnit DebugBuilder::compileUnit() const {
			const auto namedNode = module_.getLLVMModule().getNamedMetadata("llvm.dbg.cu");
			assert(namedNode != nullptr);
			const auto node = namedNode.getOperand(0);
			assert(node != nullptr);
			return llvm::DICompileUnit(node);
		}*/
		
		llvm::DIFile DebugBuilder::createFile(const std::string& fileName, const std::string& directory) {
			return builder_.createFile(fileName, directory);
		}
		
		llvm::DISubprogram DebugBuilder::convertFunction(const Name& name) {
			const auto file = createFile("testfile", "testdir");
			const auto functionType = builder_.createSubroutineType(file, builder_.getOrCreateArray(std::vector<llvm::Value*>{}));
			const auto lineNumber = 2;
			const bool isLocalToUnit = false;
			const bool isDefinition = false;
			const auto scopeLine = 4;
			return builder_.createFunction(file, name.toString(), mangleFunctionName(module_, name),
				file, lineNumber, functionType, isLocalToUnit, isDefinition, scopeLine);
		}
		
		llvm::DIVariable DebugBuilder::convertVar(SEM::Var* var, bool isParam) {
			(void) var;
			(void) isParam;
			// TODO!
			return llvm::DIVariable();
		}
		
	}
	
}

