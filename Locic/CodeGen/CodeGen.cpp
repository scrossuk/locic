#include <Locic/CodeGen/LLVMIncludes.hpp>

#include <assert.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <Locic/Log.hpp>
#include <Locic/Map.hpp>
#include <Locic/SEM.hpp>
#include <Locic/String.hpp>

#include <Locic/CodeGen/CodeGen.hpp>
#include <Locic/CodeGen/Function.hpp>
#include <Locic/CodeGen/GenFunction.hpp>
#include <Locic/CodeGen/GenStatement.hpp>
#include <Locic/CodeGen/GenType.hpp>
#include <Locic/CodeGen/GenTypeInstance.hpp>
#include <Locic/CodeGen/Mangling.hpp>
#include <Locic/CodeGen/Memory.hpp>
#include <Locic/CodeGen/Optimisations.hpp>
#include <Locic/CodeGen/Primitives.hpp>
#include <Locic/CodeGen/Support.hpp>
#include <Locic/CodeGen/TargetInfo.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>
#include <Locic/CodeGen/VTable.hpp>

namespace Locic {

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
			
			const std::vector<SEM::Function*>& functions = typeInstance->functions();
			
			// TODO: Remove this, since CodeGen should not generate any SEM trees.
			SEM::Type* objectType =
				SEM::Type::Object(SEM::Type::MUTABLE, typeInstance, std::vector<SEM::Type*>());
			
			for (size_t i = 0; i < functions.size(); i++) {
				(void) genFunction(module, objectType, functions.at(i));
			}
		}
		
		void genNamespaceFunctions(Module& module, SEM::Namespace* nameSpace) {
			const std::vector<SEM::Namespace*>& namespaces = nameSpace->namespaces();
			
			for (size_t i = 0; i < namespaces.size(); i++) {
				genNamespaceFunctions(module, namespaces.at(i));
			}
			
			const std::vector<SEM::TypeInstance*>& typeInstances = nameSpace->typeInstances();
			
			for (size_t i = 0; i < typeInstances.size(); i++) {
				genTypeInstanceFunctions(module, typeInstances.at(i));
			}
			
			const std::vector<SEM::Function*>& functions = nameSpace->functions();
			
			for (size_t i = 0; i < functions.size(); i++) {
				(void) genFunction(module, NULL, functions.at(i));
			}
		}
		
		CodeGenerator::CodeGenerator(const TargetInfo& targetInfo, const std::string& moduleName) {
			module_ = new Module(moduleName, targetInfo);
		}
		
		CodeGenerator::~CodeGenerator() {
			delete module_;
		}
		
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

