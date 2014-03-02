#include <assert.h>

#include <string>
#include <vector>

#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Function.hpp>
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
			const unsigned language = llvm::dwarf::DW_LANG_lo_user;
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
		
		llvm::DISubprogram DebugBuilder::createFunction(llvm::DIFile file, unsigned int lineNumber, bool isDefinition, const Name& name, llvm::DIType functionType, llvm::Function* function) {
			const bool isLocalToUnit = false;
			const auto scopeLine = lineNumber;
			const auto flags = llvm::DIDescriptor::FlagPrototyped;
			const bool isOptimised = false;
			
			return builder_.createFunction(file, name.toString(), "",
				file, lineNumber, functionType, isLocalToUnit, isDefinition, scopeLine,
				flags, isOptimised, function);
		}
		
		llvm::DIVariable DebugBuilder::createVar(llvm::DIDescriptor scope, bool isParam, const std::string& name, llvm::DIFile file, unsigned lineNumber, llvm::DIType type) {
			const auto tag = isParam ? llvm::dwarf::DW_TAG_arg_variable : llvm::dwarf::DW_TAG_auto_variable;
			return builder_.createLocalVariable(tag, scope, name, file, lineNumber, type);
		}
		
		llvm::DIType DebugBuilder::createVoidType() {
			return llvm::DIType();
		}
		
		llvm::DIType DebugBuilder::createNullType() {
			return builder_.createNullPtrType("null");
		}
		
		llvm::DIType DebugBuilder::createReferenceType(llvm::DIType type) {
			return builder_.createReferenceType(0, type);
		}
		
		llvm::DIType DebugBuilder::createObjectType(llvm::DIFile file, unsigned int lineNumber, const Name& name) {
			// TODO!
			const auto sizeInBits = 32;
			const auto alignInBits = 32;
			const auto flags = 0;
			const auto derivedFrom = createVoidType();
			const auto elements = llvm::DIArray();
			
			return builder_.createStructType(compileUnit(), name.toString(), file,
				lineNumber, sizeInBits, alignInBits,
				flags, derivedFrom, elements);
		}
		
		llvm::DIType DebugBuilder::createFunctionType(llvm::DIFile file, const std::vector<llvm::Value*>& parameters) {
			return builder_.createSubroutineType(file, builder_.getOrCreateArray(parameters));
		}
		
		llvm::Instruction* DebugBuilder::insertVariableDeclare(Function& function, llvm::DIVariable variable, llvm::Value* varValue) {
			return builder_.insertDeclare(varValue, variable, function.getSelectedBasicBlock());
		}
		
		namespace {
			
			std::pair<std::string, std::string> splitPath(const std::string& path) {
				for (size_t i = 0; i < path.size(); i++) {
					const auto pos = path.size() - i - 1;
					if (path.at(pos) == '/') {
						return std::make_pair(path.substr(0, pos), path.substr(pos + 1, i));
					}
				}
				return std::make_pair("", path);
			}
			
		}
		
		llvm::Instruction* genDebugVar(Function& function, const SourceLocation& sourceLocation, bool isParam, const std::string& name, llvm::DIType type, llvm::Value* varValue) {
			auto& module = function.getModule();
			const auto components = splitPath(sourceLocation.fileName());
			const auto file = module.debugBuilder().createFile(components.second, components.first);
			const auto lineNumber = 1;
			const auto varDebugInfo = module.debugBuilder().createVar(function.debugInfo(), isParam, name, file, lineNumber, type);
			return module.debugBuilder().insertVariableDeclare(function, varDebugInfo, varValue);
		}
		
	}
	
}

