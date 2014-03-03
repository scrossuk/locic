#include <locic/CodeGen/LLVMIncludes.hpp>

#include <assert.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <locic/Debug.hpp>
#include <locic/Log.hpp>
#include <locic/Map.hpp>
#include <locic/SEM.hpp>
#include <locic/String.hpp>

#include <locic/CodeGen/CodeGen.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenStatement.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenTypeInstance.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Optimisations.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TargetInfo.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
	
		void genNamespaceTypes(Module& module, SEM::Namespace* nameSpace) {
			const std::vector<SEM::Namespace*>& namespaces = nameSpace->namespaces();
			
			for (size_t i = 0; i < namespaces.size(); i++) {
				genNamespaceTypes(module, namespaces.at(i));
			}
			
			const std::vector<SEM::TypeInstance*>& typeInstances = nameSpace->typeInstances();
			
			for (size_t i = 0; i < typeInstances.size(); i++) {
				SEM::TypeInstance* typeInstance = typeInstances.at(i);
				
				if (!typeInstance->templateVariables().empty()) {
					// Can't generate types with template arguments.
					return;
				}
				
				if (typeInstance->isPrimitive()) {
					// Can't generate primitive types.
					return;
				}
				
				if (typeInstance->isInterface()) {
					// Can't generate interface types.
					return;
				}
				
				(void) genTypeInstance(module, typeInstance, std::vector<SEM::Type*>());
			}
		}
		
		void genTypeInstanceFunctions(Module& module, SEM::TypeInstance* typeInstance) {
			if (!typeInstance->templateVariables().empty()) {
				// Can't generate types with template arguments.
				return;
			}
			
			if (typeInstance->isInterface()) {
				// Can't generate interface types.
				return;
			}
			
			const auto& functions = typeInstance->functions();
			
			// TODO: Remove this, since CodeGen should not generate any SEM trees.
			const auto objectType = SEM::Type::Object(typeInstance, SEM::Type::NO_TEMPLATE_ARGS);
			
			for (size_t i = 0; i < functions.size(); i++) {
				(void) genFunction(module, objectType, functions.at(i));
			}
		}
		
		void genNamespaceFunctions(Module& module, SEM::Namespace* nameSpace) {
			for (const auto childNamespace: nameSpace->namespaces()) {
				genNamespaceFunctions(module, childNamespace);
			}
			
			for (const auto typeInstance: nameSpace->typeInstances()) {
				genTypeInstanceFunctions(module, typeInstance);
			}
			
			for (const auto function: nameSpace->functions()) {
				const auto parent = nullptr;
				(void) genFunction(module, parent, function);
			}
		}
		
		CodeGenerator::CodeGenerator(const TargetInfo& targetInfo, const std::string& moduleName, Debug::Module& debugModule)
			: module_(new Module(moduleName, targetInfo, debugModule)) {
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
			genNamespaceTypes(*module_, nameSpace);
			genNamespaceFunctions(*module_, nameSpace);
			
			module_->debugBuilder().finalize();
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

