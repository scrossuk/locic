#include <assert.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <locic/Debug.hpp>
#include <locic/Support/Map.hpp>
#include <locic/SEM.hpp>
#include <locic/Support/String.hpp>

#include <locic/CodeGen/CodeGenerator.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Context.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenStatement.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenTypeInstance.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/ModulePtr.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Optimisations.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
	
		void genNamespaceTypes(Module& module, const SEM::Namespace& nameSpace) {
			for (const auto& itemPair: nameSpace.items()) {
				const auto& item = itemPair.second;
				if (item.isNamespace()) {
					genNamespaceTypes(module, item.nameSpace());
				} else if (item.isTypeInstance()) {
					const auto& typeInstance = item.typeInstance();
					
					if (typeInstance.isPrimitive()) {
						// Can't generate primitive types.
						continue;
					}
					
					if (typeInstance.isInterface()) {
						// Can't generate interface types.
						continue;
					}
					
					(void) genTypeInstance(module, &typeInstance);
				}
			}
		}
		
		void genTypeInstanceFunctions(Module& module, const SEM::TypeInstance& typeInstance) {
			if (typeInstance.isInterface()) {
				// Can't generate interface types.
				return;
			}
			
			if (typeInstance.isOpaqueStruct()) {
				// Opaque structs don't have any methods.
				assert(typeInstance.functions().empty());
				return;
			}
			
			const auto& functions = typeInstance.functions();
			
			for (const auto& functionPair: functions) {
				const auto& function = functionPair.second;
				
				if (function->isPrimitive()) {
					// Only generate 'primitive' functions when needed.
					continue;
				}
				
				if (function->requiresPredicate().isFalse()) {
					// Don't generate functions that are always invalid.
					continue;
				}
				
				(void) genFunctionDef(module, &typeInstance, function.get());
			}
			
			// Only generate primitives as needed.
			if (!typeInstance.isPrimitive()) {
				(void) genMoveFunctionDef(module, &typeInstance);
				(void) genDestructorFunctionDef(module, typeInstance);
				(void) genAlignMaskFunctionDef(module, &typeInstance);
				(void) genSizeOfFunctionDef(module, &typeInstance);
				(void) genSetDeadDefaultFunctionDef(module, &typeInstance);
				(void) genIsLiveDefaultFunctionDef(module, &typeInstance);
				
				if (!typeInstance.templateVariables().empty()) {
					auto& templateBuilder = module.templateBuilder(TemplatedObject::TypeInstance(&typeInstance));
					(void) genTemplateIntermediateFunction(module, TemplatedObject::TypeInstance(&typeInstance), templateBuilder);
					
					// Update all instructions needing the bits required value
					// with the correct value (now it is known).
					templateBuilder.updateAllInstructions(module);
				}
			}
		}
		
		void genNamespaceFunctions(Module& module, const SEM::Namespace& nameSpace) {
			for (const auto& itemPair: nameSpace.items()) {
				const auto& item = itemPair.second;
				if (item.isFunction()) {
					const auto parent = nullptr;
					(void) genFunctionDef(module, parent, &(item.function()));
				} else if (item.isTypeInstance()) {
					genTypeInstanceFunctions(module, item.typeInstance());
				} else if (item.isNamespace()) {
					genNamespaceFunctions(module, item.nameSpace());
				}
			}
		}
		
		CodeGenerator::CodeGenerator(Context& context, const std::string& moduleName, Debug::Module& debugModule, const BuildOptions& buildOptions)
			: module_(new Module(context.internal(), moduleName, debugModule, buildOptions)) {
			// TODO: fill these in correctly.
			DebugCompileUnit compileUnit;
			compileUnit.compilerName = "Loci Compiler";
			compileUnit.directoryName = "SOMEDIR";
			compileUnit.fileName = "SOMEFILE";
			compileUnit.flags = "example_compiler_flags";
			
			module_->debugBuilder().createCompileUnit(compileUnit);
		}
		
		CodeGenerator::~CodeGenerator() { }
		
		Module& CodeGenerator::module() {
			return *module_;
		}
		
		ModulePtr CodeGenerator::releaseModule() {
			auto releasedValue = std::move(module_);
			module_ = std::unique_ptr<Module>();
			return std::move(releasedValue);
		}
		
		void CodeGenerator::applyOptimisations(size_t optLevel) {
			if (optLevel == 0) {
				// Don't touch the code if optimisations
				// are set to zero; useful for debugging.
				return;
			}
			Optimisations optimisations(*module_);
			optimisations.addDefaultPasses(optLevel);
			optimisations.run();
		}
		
		void CodeGenerator::genNamespace(SEM::Namespace* nameSpace) {
			genNamespaceTypes(*module_, *nameSpace);
			genNamespaceFunctions(*module_, *nameSpace);
			module_->debugBuilder().finalize();
			module_->verify();
		}
		
		void CodeGenerator::writeToFile(const std::string& fileName) {
			module_->writeBitCodeToFile(fileName);
		}
		
		void CodeGenerator::dumpToFile(const std::string& fileName) {
			module_->dumpToFile(fileName);
		}
		
		void CodeGenerator::dump() {
			module_->dump();
		}
		
	}
	
}

