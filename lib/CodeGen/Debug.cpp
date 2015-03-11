#include <assert.h>

#include <string>
#include <vector>

#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/SEM/Function.hpp>
#include <locic/SEM/Value.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/Optional.hpp>

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
		
		llvm::DISubprogram DebugBuilder::createFunction(llvm::DIFile file, const unsigned int lineNumber,
				const bool isInternal, const bool isDefinition, const Name& name,
				llvm::DIType functionType, llvm::Function* const function) {
			const bool isLocalToUnit = isInternal;
			const auto scopeLine = lineNumber;
			const auto flags = llvm::DIDescriptor::FlagPrototyped;
			const bool isOptimised = false;
			
			return builder_.createFunction(file, name.toString(false), "",
				file, lineNumber, llvm::DICompositeType(functionType),
				isLocalToUnit, isDefinition, scopeLine,
				flags, isOptimised, function);
		}
		
		llvm::DIVariable DebugBuilder::createVar(llvm::DIDescriptor scope, bool isParam, const String& name, llvm::DIFile file, unsigned lineNumber, llvm::DIType type) {
			const auto tag = isParam ? llvm::dwarf::DW_TAG_arg_variable : llvm::dwarf::DW_TAG_auto_variable;
			return builder_.createLocalVariable(tag, scope, name.c_str(), file, lineNumber, type);
		}
		
		llvm::DIType DebugBuilder::createUnspecifiedType(const String& name) {
			return builder_.createUnspecifiedType(name.c_str());
		}
		
		llvm::DIType DebugBuilder::createVoidType() {
			return llvm::DIType();
		}
		
		llvm::DIType DebugBuilder::createNullType() {
#if defined(LLVM_3_3)
			return builder_.createNullPtrType("null");
#else
			return builder_.createNullPtrType();
#endif
		}
		
		llvm::DIType DebugBuilder::createReferenceType(llvm::DIType type) {
			return builder_.createReferenceType(llvm::dwarf::DW_TAG_reference_type, type);
		}
		
		llvm::DIType DebugBuilder::createPointerType(llvm::DIType type) {
			const auto pointerSize = module_.abi().typeSize(llvm_abi::Type::Pointer(module_.abiContext()));
			return builder_.createPointerType(type, pointerSize);
		}
		
		llvm::DIType DebugBuilder::createIntType(const String& name) {
			const auto& abi = module_.abi();
			const auto abiType = getNamedPrimitiveABIType(module_, name);
			return builder_.createBasicType(name.c_str(), abi.typeSize(abiType), abi.typeAlign(abiType), llvm::dwarf::DW_ATE_signed);
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
		
		llvm::DIType DebugBuilder::createFunctionType(llvm::DIFile file, const std::vector<LLVMMetadataValue*>& parameters) {
#if defined(LLVM_3_6)
			return builder_.createSubroutineType(file, builder_.getOrCreateTypeArray(parameters));
#else
			return builder_.createSubroutineType(file, builder_.getOrCreateArray(parameters));
#endif
		}
		
		llvm::Instruction* DebugBuilder::insertVariableDeclare(Function& function, llvm::DIVariable variable, llvm::Value* varValue) {
#if defined(LLVM_3_6)
			const auto declareInstruction = builder_.insertDeclare(varValue, variable, builder_.createExpression(), function.getEntryBuilder().GetInsertPoint());
#else
			const auto declareInstruction = builder_.insertDeclare(varValue, variable, function.getEntryBuilder().GetInsertPoint());
#endif
			return declareInstruction;
		}
		
		llvm::DISubprogram genDebugFunction(Module& module, const Debug::FunctionInfo& functionInfo, llvm::DIType functionType, llvm::Function* function, const bool isInternal) {
			const auto file = module.debugBuilder().createFile(functionInfo.declLocation.fileName());
			const auto lineNumber = functionInfo.declLocation.range().start().lineNumber();
			return module.debugBuilder().createFunction(file, lineNumber, isInternal,
				functionInfo.isDefinition, functionInfo.name, functionType, function);
		}
		
		Optional<llvm::DISubprogram> genDebugFunctionInfo(Module& module, SEM::Function* const function, llvm::Function* const llvmFunction) {
			const auto debugInfo = function->debugInfo();
			
			if (debugInfo) {
				const auto debugSubprogramType = genDebugType(module, function->type());
				const auto& functionInfo = *debugInfo;
				const bool isInternal = function->moduleScope().isInternal();
				return make_optional(genDebugFunction(module, functionInfo, debugSubprogramType, llvmFunction, isInternal));
			} else {
				return None;
			}
		}
		
		llvm::Instruction* genDebugVar(Function& function, const Debug::VarInfo& varInfo, llvm::DIType type, llvm::Value* varValue) {
			assert(type.isType());
			auto& module = function.module();
			const auto file = module.debugBuilder().createFile(varInfo.declLocation.fileName());
			const auto lineNumber = varInfo.declLocation.range().start().lineNumber();
			const bool isParam = (varInfo.kind == Debug::VarInfo::VAR_ARG);
			
			const auto varDebugInfo = module.debugBuilder().createVar(function.debugInfo(), isParam, varInfo.name, file, lineNumber, type);
			return module.debugBuilder().insertVariableDeclare(function, varDebugInfo, varValue);
		}
		
		llvm::DebugLoc getDebugLocation(Function& function, const Debug::SourceLocation& debugSourceLocation) {
			const auto debugStartPosition = debugSourceLocation.range().start();
			return llvm::DebugLoc::get(debugStartPosition.lineNumber(), debugStartPosition.column(), function.debugInfo());
		}
		
		Optional<llvm::DebugLoc> getFunctionDebugLocation(Function& function, const SEM::Function& semFunction) {
			const auto debugInfo = semFunction.debugInfo();
			if (debugInfo) {
				const auto debugSourceLocation = debugInfo->declLocation;
				return make_optional(getDebugLocation(function, debugSourceLocation));
			} else {
				return None;
			}
		}
		
		Optional<llvm::DebugLoc> getValueDebugLocation(Function& function, const SEM::Value& value) {
			const auto debugInfo = value.debugInfo();
			if (debugInfo) {
				const auto debugSourceLocation = debugInfo->location;
				return make_optional(getDebugLocation(function, debugSourceLocation));
			} else {
				return None;
			}
		}
		
	}
	
}

