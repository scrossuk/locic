#include <llvm/Attributes.h>
#include <llvm/DerivedTypes.h>
#include <llvm/InlineAsm.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Support/Host.h>

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
			if (!typeInstance->templateVariables().empty() && !typeInstance->isPrimitive()) {
				// Can't generate types with template arguments.
				return;
			}
			
			if (typeInstance->isInterface()) {
				// Can't generate interface types.
				return;
			}
			
			const std::vector<SEM::Function*>& functions = typeInstance->functions();
			
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
		
		class InternalCodeGen {
			private:
				std::string name_;
				Module module_;
				
			public:
				InternalCodeGen(const std::string& moduleName, const TargetInfo& targetInfo)
					: module_(moduleName, targetInfo) { }
				
				Module& getModule() {
					return module_;
				}
				
				// ---- Pass 4: Generate function code.
				
				// TODO: this needs to be removed...
				/*void genInterfaceMethod(SEM::Function* function) {
					assert(function->isMethod());
					
					if (function->isStatic()) {
						// Don't process static methods of interfaces.
						return;
					}
					
					assert(function->isDeclaration() && "Interface methods must be declarations");
					
					LOG(LOG_INFO, "Generating interface method '%s'.",
						function->name().toString().c_str());
						
					llvm::Function* llvmFunction = module_.getFunctionMap().get(function);
					
					Function genFunction(module_, llvmFunction, getArgInfo(function));
					
					// Store arguments onto stack.
					llvm::Function::arg_iterator arg = generatedFunction->arg_begin();
					SEM::Type* returnType = function->type()->getFunctionReturnType();
					llvm::Value* returnVar = returnType->isClass() ? arg++ : NULL;
					
					// Get the 'this' record, which is the
					// pair of the 'this' pointer and the
					// method vtable pointer.
					llvm::Value* thisRecord = arg++;
					
					// Get the 'this' pointer.
					llvm::Value* thisPointer = function.getBuilder().CreateExtractValue(thisRecord, std::vector<unsigned>(1, 0), "thisPointer");
					
					// Get the vtable pointer.
					llvm::Value* vtablePointer = function.getBuilder().CreateExtractValue(thisRecord,
												 std::vector<unsigned>(1, 1), "vtablePointer");
												 
					const MethodHash methodHash = CreateMethodNameHash(function->name().last());
					const size_t offset = methodHash % VTABLE_SIZE;
					
					std::vector<llvm::Value*> vtableEntryGEP;
					vtableEntryGEP.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, 0)));
					vtableEntryGEP.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, 2)));
					vtableEntryGEP.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, offset)));
					
					llvm::Value* vtableEntryPointer = function.getBuilder().CreateInBoundsGEP(vtablePointer, vtableEntryGEP, "vtableEntryPointer");
					llvm::Value* methodFunctionPointer = function.getBuilder().CreateLoad(vtableEntryPointer, "methodFunctionPointer");
					llvm::Type* methodFunctionType = genFunctionType(function->type(), thisPointer->getType());
					llvm::Value* castedMethodFunctionPointer = function.getBuilder().CreatePointerCast(
								methodFunctionPointer, methodFunctionType->getPointerTo(), "castedMethodFunctionPointer");
					std::vector<llvm::Value*> arguments;
					
					if (returnVar != NULL) {
						arguments.push_back(returnVar);
					}
					
					arguments.push_back(thisPointer);
					
					while (arg != generatedFunction->arg_end()) {
						arguments.push_back(arg++);
					}
					
					llvm::FunctionType* asmFunctionType = llvm::FunctionType::get(voidType(), std::vector<llvm::Type*>(), false);
					const std::string assembly = makeString("movl $$%llu, %%eax",
															(unsigned long long) methodHash);
															
					llvm::InlineAsm* setEax = llvm::InlineAsm::get(asmFunctionType, assembly, "~eax", true);
					function.getBuilder().CreateCall(setEax);
					
					const bool isVoidReturnType = returnType->isVoid() || returnType->isClassOrTemplateVar();
					
					llvm::Value* methodCallValue = function.getBuilder().CreateCall(castedMethodFunctionPointer,
												   arguments, isVoidReturnType ? "" : "methodCallValue");
												   
					if (isVoidReturnType) {
						function.getBuilder().CreateRetVoid();
					} else {
						function.getBuilder().CreateRet(methodCallValue);
					}
					
					// Check the generated function is correct.
					genFunction.verify();
				}*/
				
		};
		
		CodeGenerator::CodeGenerator(const TargetInfo& targetInfo, const std::string& moduleName) {
			module_ = new Module(moduleName, targetInfo);
		}
		
		CodeGenerator::~CodeGenerator() {
			delete module_;
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

