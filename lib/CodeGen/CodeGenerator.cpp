#include <assert.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <locic/AST/Function.hpp>
#include <locic/AST/Namespace.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/Debug.hpp>

#include <locic/Support/Map.hpp>
#include <locic/Support/String.hpp>

#include <locic/CodeGen/CodeGenerator.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Context.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/ModulePtr.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Optimisations.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SEMFunctionGenerator.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {
	
	namespace CodeGen {
		
		void genTypeInstanceFunctions(Module& module, const AST::TypeInstance& typeInstance) {
			if (typeInstance.isInterface()) {
				// Can't generate interface types.
				return;
			}
			
			if (typeInstance.isOpaqueStruct()) {
				// Opaque structs don't have any methods.
				assert(typeInstance.functions().empty());
				return;
			}
			
			auto& semFunctionGenerator = module.semFunctionGenerator();
			
			const auto& functions = typeInstance.functions();
			
			for (const auto& function: functions) {
				if (function->isPrimitive()) {
					// Only generate 'primitive' functions when needed.
					continue;
				}
				
				if (function->requiresPredicate().isFalse()) {
					// Don't generate functions that are always invalid.
					continue;
				}
				
				(void) semFunctionGenerator.genDef(&typeInstance,
				                                   *function);
			}
			
			if (typeInstance.isPrimitive()) {
				// Only generate primitives as needed.
				return;
			}
			
			if (!typeInstance.templateVariables().empty()) {
				auto& templateBuilder = module.templateBuilder(TemplatedObject::TypeInstance(&typeInstance));
				(void) genTemplateIntermediateFunction(module, TemplatedObject::TypeInstance(&typeInstance), templateBuilder);
				
				// Update all instructions needing the bits required value
				// with the correct value (now it is known).
				templateBuilder.updateAllInstructions(module);
			}
		}
		
		void genNamespaceFunctions(Module& module, const AST::Namespace& nameSpace) {
			for (const auto& itemPair: nameSpace.items()) {
				const auto& item = itemPair.second;
				if (item.isFunction()) {
					if (item.function().isPrimitive()) {
						// Only generate 'primitive' functions when needed.
						continue;
					}
					auto& semFunctionGenerator = module.semFunctionGenerator();
					(void) semFunctionGenerator.genDef(/*parent=*/nullptr,
					                                   item.function());
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
		
		void CodeGenerator::genNamespace(AST::Namespace* nameSpace) {
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

