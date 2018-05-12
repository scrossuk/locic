#include <locic/CodeGen/Debug.hpp>

#include <cassert>
#include <string>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Function.hpp>
#include <locic/AST/Value.hpp>

#include <locic/CodeGen/ASTFunctionGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenDebugType.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>

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
		
		DICompileUnit DebugBuilder::createCompileUnit(const DebugCompileUnit& compileUnitInfo) {
			const unsigned language = llvm::dwarf::DW_LANG_lo_user;
			const bool isOptimized = false;
			const unsigned runtimeVersion = 0;
			const auto file = builder_.createFile(compileUnitInfo.fileName, compileUnitInfo.directoryName);
			builder_.createCompileUnit(language,
			                           file,
			                           compileUnitInfo.compilerName,
			                           isOptimized,
			                           compileUnitInfo.flags,
			                           runtimeVersion);
			return compileUnit();
		}
		
		DICompileUnit DebugBuilder::compileUnit() const {
			const auto namedNode = module_.getLLVMModule().getNamedMetadata("llvm.dbg.cu");
			assert(namedNode != nullptr);
			const auto node = namedNode->getOperand(0);
			assert(node != nullptr);
			return DICompileUnit(node);
		}
		
		DIFile DebugBuilder::createFile(const std::string& path) {
			const auto components = splitPath(path);
			return builder_.createFile(components.second, components.first);
		}
		
		DISubprogram DebugBuilder::createFunction(DIScope scope,
		                                          const unsigned int lineNumber,
		                                          const bool isInternal,
		                                          const bool isDefinition,
		                                          const Name& name,
		                                          DISubroutineType functionType,
		                                          llvm::Function* const function) {
			assert(function != nullptr);
			const bool isLocalToUnit = isInternal;
			const auto scopeLine = lineNumber;
			const auto flags = DINode::FlagPrototyped;
			const bool isOptimised = false;
			
			return builder_.createFunction(scope, name.toString(false), "",
				scope->getFile(), lineNumber, functionType,
				isLocalToUnit, isDefinition, scopeLine,
				flags, isOptimised);
		}
		
		DILocalVariable DebugBuilder::createVar(DIScope scope,
		                                        bool isParam,
		                                        const String& name,
		                                        DIFile file,
		                                        unsigned lineNumber,
		                                        DIType type,
		                                        const size_t argIndex) {
			if (isParam) {
				return builder_.createParameterVariable(scope,
				                                        name.c_str(),
				                                        argIndex,
				                                        file,
				                                        lineNumber,
				                                        type);
			} else {
				return builder_.createAutoVariable(scope, name.c_str(),
				                                   file, lineNumber,
				                                   type);
			}
		}
		
		DIType DebugBuilder::createUnspecifiedType(const String& name) {
			return builder_.createUnspecifiedType(name.c_str());
		}
		
		DIType DebugBuilder::createVoidType() {
			return builder_.createUnspecifiedType("void");
		}
		
		DIType DebugBuilder::createNullType() {
			return builder_.createNullPtrType();
		}
		
		DIType DebugBuilder::createReferenceType(DIType type) {
			return builder_.createReferenceType(llvm::dwarf::DW_TAG_reference_type, type);
		}
		
		DIType DebugBuilder::createPointerType(DIType type) {
			const auto& abiTypeInfo = module_.abi().typeInfo();
			const auto pointerSize = abiTypeInfo.getTypeRawSize(module_.abiTypeBuilder().getPointerTy());
			return builder_.createPointerType(type, pointerSize.asBits());
		}
		
		DIType DebugBuilder::createIntType(const PrimitiveID primitiveID) {
			assert(primitiveID.isInteger());
			const auto& abi = module_.abi();
			const auto abiType = getBasicPrimitiveABIType(module_, primitiveID);
			const auto encoding = primitiveID.isSignedInteger() ?
			                      llvm::dwarf::DW_ATE_signed :
			                      llvm::dwarf::DW_ATE_unsigned;
			return builder_.createBasicType(primitiveID.toCString(),
			                                abi.typeInfo().getTypeRawSize(abiType).asBits(),
			                                encoding);
		}
		
		DIType DebugBuilder::createObjectType(DIFile file,
		                                      unsigned int lineNumber,
		                                      const Name& name,
		                                      const size_t sizeInBits,
		                                      const size_t alignInBits) {
			// TODO!
			const auto offsetInBits = 0;
			const auto flags = llvm::DINode::FlagZero;
			const auto derivedFrom = createVoidType();
			const auto elements = builder_.getOrCreateArray({});
			
			return builder_.createClassType(compileUnit(), name.toString(false), file,
				lineNumber, sizeInBits, alignInBits, offsetInBits,
				flags, derivedFrom, elements);
		}
		
		DISubroutineType DebugBuilder::createFunctionType(const std::vector<LLVMMetadataValue*>& parameters) {
			return builder_.createSubroutineType(builder_.getOrCreateTypeArray(parameters));
		}
		
		llvm::Instruction* DebugBuilder::insertVariableDeclare(Function& function,
		                                                       DILocalVariable variable,
		                                                       llvm::DebugLoc location,
		                                                       llvm::Value* varValue) {
			return builder_.insertDeclare(varValue,
			                              variable,
			                              builder_.createExpression(),
			                              location,
			                              function.getEntryBuilder().GetInsertBlock());
		}
		
		DISubprogram genDebugFunction(Module& module,
		                              const Debug::FunctionInfo& functionInfo,
		                              DISubroutineType functionType,
		                              llvm::Function* function,
		                              const bool isInternal,
		                              const bool isDefinition) {
			const auto file = module.debugBuilder().createFile(functionInfo.declLocation.fileName().asStdString());
			const auto lineNumber = functionInfo.declLocation.range().start().lineNumber();
			return module.debugBuilder().createFunction(file,
			                                            lineNumber,
			                                            isInternal,
			                                            isDefinition,
			                                            functionInfo.name,
			                                            functionType,
			                                            function);
		}
		
		Optional<DISubprogram> genDebugFunctionInfo(Module& module,
		                                            const AST::TypeInstance* parentType,
		                                            const AST::Function& function,
		                                            llvm::Function* const llvmFunction) {
			const auto& debugInfo = function.debugInfo();
			
			if (debugInfo) {
				const auto debugSubprogramType = genDebugFunctionType(module, function.type());
				const auto& functionInfo = *debugInfo;
				const bool isInternal = function.moduleScope().isInternal();
				const bool isDefinition = ASTFunctionGenerator(module).hasDef(parentType,
				                                                              function);
				return make_optional(genDebugFunction(module, functionInfo, debugSubprogramType, llvmFunction, isInternal, isDefinition));
			} else {
				return None;
			}
		}
		
		llvm::Instruction* genDebugVar(Function& function,
		                               const Debug::VarInfo& varInfo,
		                               DIType type,
		                               llvm::Value* varValue,
		                               const size_t argIndex) {
			auto& module = function.module();
			const auto file = module.debugBuilder().createFile(varInfo.declLocation.fileName().asStdString());
			const auto lineNumber = varInfo.declLocation.range().start().lineNumber();
			const bool isParam = (varInfo.kind == Debug::VarInfo::VAR_ARGUMENT);
			
			const auto location = getDebugLocation(function, varInfo.declLocation);
			const auto varDebugInfo =
				module.debugBuilder().createVar(function.debugInfo(), isParam,
				                                varInfo.name, file, lineNumber,
				                                type, argIndex);
			return module.debugBuilder().insertVariableDeclare(function, varDebugInfo, location, varValue);
		}
		
		llvm::DebugLoc getDebugLocation(Function& function,
		                                const Debug::SourceLocation& debugSourceLocation) {
			const auto debugStartPosition = debugSourceLocation.range().start();
			return llvm::DebugLoc::get(debugStartPosition.lineNumber(), debugStartPosition.column(), function.debugInfo());
		}
		
		Optional<llvm::DebugLoc> getFunctionDebugLocation(Function& function,
		                                                  const AST::Function& astFunction) {
			const auto& debugInfo = astFunction.debugInfo();
			if (debugInfo) {
				const auto debugSourceLocation = debugInfo->declLocation;
				return make_optional(getDebugLocation(function, debugSourceLocation));
			} else {
				return None;
			}
		}
		
	}
	
}

