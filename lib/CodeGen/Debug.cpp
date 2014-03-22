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
	
		std::pair<std::string, std::string> splitPath(const std::string& path) {
			for (size_t i = 0; i < path.size(); i++) {
				const auto pos = path.size() - i - 1;
				if (path.at(pos) == '/') {
					return std::make_pair(path.substr(0, pos), path.substr(pos + 1, i));
				}
			}
			return std::make_pair("", path);
		}
		
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
		
		llvm::DIFile DebugBuilder::createFile(const std::string& path) {
			const auto components = splitPath(path);
			return builder_.createFile(components.second, components.first);
		}
		
		llvm::DISubprogram DebugBuilder::createFunction(llvm::DIFile file, unsigned int lineNumber, bool isDefinition, const Name& name, llvm::DIType functionType, llvm::Function* function) {
			const bool isLocalToUnit = false;
			const auto scopeLine = lineNumber;
			const auto flags = llvm::DIDescriptor::FlagPrototyped;
			const bool isOptimised = false;
			
			return builder_.createFunction(file, name.toString(false), "",
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
		
		llvm::DIType DebugBuilder::createPointerType(llvm::DIType type) {
			const auto& targetInfo = module_.getTargetInfo();
			return builder_.createPointerType(type, targetInfo.getPointerSize());
		}
		
		llvm::DIType DebugBuilder::createIntType(const std::string& name) {
			const auto& targetInfo = module_.getTargetInfo();
			return builder_.createBasicType(name, targetInfo.getPrimitiveSize(name),
				targetInfo.getPrimitiveAlign(name), llvm::dwarf::DW_ATE_signed);
		}
		
		llvm::DIType DebugBuilder::createObjectType(llvm::DIFile file, unsigned int lineNumber, const Name& name) {
			// TODO!
			const auto sizeInBits = 32;
			const auto alignInBits = 32;
			const auto offsetInBits = 0;
			const auto flags = 0;
			const auto derivedFrom = createVoidType();
			const auto elements = llvm::DIArray();
			
			return builder_.createClassType(compileUnit(), name.toString(false), file,
				lineNumber, sizeInBits, alignInBits, offsetInBits,
				flags, derivedFrom, elements);
		}
		
		llvm::DIType DebugBuilder::createFunctionType(llvm::DIFile file, const std::vector<llvm::Value*>& parameters) {
			return builder_.createSubroutineType(file, builder_.getOrCreateArray(parameters));
		}
		
		llvm::Instruction* DebugBuilder::insertVariableDeclare(Function& function, llvm::DIVariable variable, llvm::Value* varValue) {
			const auto dbgDeclareIntrinsic = llvm::Intrinsic::getDeclaration(function.module().getLLVMModulePtr(), llvm::Intrinsic::dbg_declare);
			llvm::Value* args[] = { llvm::MDNode::get(varValue->getContext(), varValue), variable };
			return function.getEntryBuilder().CreateCall(dbgDeclareIntrinsic, args);
		}
		
		llvm::DISubprogram genDebugFunction(Module& module, const Debug::FunctionInfo& functionInfo, llvm::DIType functionType, llvm::Function* function) {
			const auto file = module.debugBuilder().createFile(functionInfo.declLocation.fileName());
			const auto lineNumber = functionInfo.declLocation.range().start().lineNumber();
			return module.debugBuilder().createFunction(file, lineNumber, functionInfo.isDefinition,
				functionInfo.name, functionType, function);
		}
		
		llvm::Instruction* genDebugVar(Function& function, const Debug::VarInfo& varInfo, llvm::DIType type, llvm::Value* varValue) {
			auto& module = function.module();
			const auto file = module.debugBuilder().createFile(varInfo.declLocation.fileName());
			const auto lineNumber = varInfo.declLocation.range().start().lineNumber();
			const bool isParam = (varInfo.kind == Debug::VarInfo::VAR_ARG);
			
			const auto varDebugInfo = module.debugBuilder().createVar(function.debugInfo(), isParam, varInfo.name, file, lineNumber, type);
			return module.debugBuilder().insertVariableDeclare(function, varDebugInfo, varValue);
		}
		
	}
	
}

