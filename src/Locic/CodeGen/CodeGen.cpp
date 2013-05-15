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
#include <Locic/CodeGen/GenStatement.hpp>
#include <Locic/CodeGen/GenType.hpp>
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
					
				llvm::GlobalValue::LinkageTypes getFunctionLinkage(SEM::TypeInstance* type) {
					if (type == NULL) {
						return llvm::Function::ExternalLinkage;
					}
					
					return type->isClass()
						   ? llvm::Function::ExternalLinkage
						   : llvm::Function::LinkOnceODRLinkage;
				}
				
				// ---- Pass 1: Generate type placeholders.
				
				void genTypeInstanceTypePlaceholders(SEM::TypeInstance* typeInstance) {
					assert(typeInstance != NULL);
					
					if (typeInstance->isPrimitive()) {
						// Skip.
						return;
					}
					
					llvm::StructType* structType = TypeGenerator(module_).getForwardDeclaredStructType(
						mangleTypeName(typeInstance->name()));
					
					module_.getTypeMap().insert(typeInstance, structType);
				}
				
				void genNamespaceTypePlaceholders(SEM::Namespace* nameSpace) {
					const std::vector<SEM::Namespace*>& namespaces = nameSpace->namespaces();
					
					for (size_t i = 0; i < namespaces.size(); i++) {
						genNamespaceTypePlaceholders(namespaces.at(i));
					}
					
					const std::vector<SEM::TypeInstance*>& typeInstances = nameSpace->typeInstances();
					
					for (size_t i = 0; i < typeInstances.size(); i++) {
						genTypeInstanceTypePlaceholders(typeInstances.at(i));
					}
				}
				
				// ---- Pass 2: Generate type members.
				
				void genTypeInstanceTypeMembers(SEM::TypeInstance* typeInstance) {
					assert(typeInstance != NULL);
					
					if (typeInstance->isPrimitive()) {
						// Skip.
						return;
					}
					
					// Generate type member variables.
					llvm::StructType* structType = module_.getTypeMap().get(typeInstance);
					
					if (typeInstance->isClassDef() || typeInstance->isStructDef()) {
						// Generating the type for a class or struct definition, so
						// the size and contents of the type instance is known and
						// hence the contents can be specified.
						std::vector<llvm::Type*> structVariables;
						
						// Add member variables.
						const std::vector<SEM::Var*>& variables = typeInstance->variables();
						
						for (size_t i = 0; i < variables.size(); i++) {
							SEM::Var* var = variables.at(i);
							structVariables.push_back(genType(module_, var->type()));
							module_.getMemberVarMap().insert(var, i);
						}
						
						LOG(LOG_INFO, "Set %llu struct variables for type '%s'.",
							(unsigned long long) structVariables.size(), typeInstance->name().toString().c_str());
							
						structType->setBody(structVariables);
					}
				}
				
				void genNamespaceTypeMembers(SEM::Namespace* nameSpace) {
					const std::vector<SEM::Namespace*>& namespaces = nameSpace->namespaces();
					
					for (size_t i = 0; i < namespaces.size(); i++) {
						genNamespaceTypeMembers(namespaces.at(i));
					}
					
					const std::vector<SEM::TypeInstance*>& typeInstances = nameSpace->typeInstances();
					
					for (size_t i = 0; i < typeInstances.size(); i++) {
						genTypeInstanceTypeMembers(typeInstances.at(i));
					}
				}
				
				// ---- Pass 3: Generate function declarations.
				
				void genFunctionDecl(SEM::TypeInstance* parent, SEM::Function* function) {
					if (function->isMethod()) {
						assert(parent != NULL);
					}
					
					LOG(LOG_INFO, "Generating %s.",
						function->name().toString().c_str());
						
					llvm::Type* contextPtrType =
						function->isMethod() && !function->isStatic() ?
						getTypeInstancePointer(module_, parent) :
						NULL;
					
					llvm::FunctionType* functionType =
						genFunctionType(module_, function->type(), contextPtrType);
						
					llvm::Function* llvmFunction =
						createLLVMFunction(module_,
							functionType, getFunctionLinkage(parent),
							mangleFunctionName(function->name()));
										   
					if (function->type()->getFunctionReturnType()->isClass()) {
						std::vector<llvm::Attributes::AttrVal> attributes;
						
						// Class return values are allocated by the caller,
						// which passes a pointer to the callee. The caller
						// and callee must, for the sake of optimisation,
						// ensure that the following attributes hold...
						
						// Caller must ensure pointer is always valid.
						attributes.push_back(llvm::Attributes::StructRet);
						
						// Caller must ensure pointer does not alias with
						// any other arguments.
						attributes.push_back(llvm::Attributes::NoAlias);
						
						// Callee must not capture the pointer.
						attributes.push_back(llvm::Attributes::NoCapture);
						
						llvmFunction->addAttribute(1,
												   llvm::Attributes::get(module_.getLLVMContext(),
														   attributes));
					}
					
					module_.getFunctionMap().insert(function, llvmFunction);
				}
				
				void genTypeInstanceFunctionDecls(SEM::TypeInstance* typeInstance) {
					const std::vector<SEM::Function*>& functions = typeInstance->functions();
					
					for (size_t i = 0; i < functions.size(); i++) {
						genFunctionDecl(typeInstance, functions.at(i));
					}
				}
				
				void genNamespaceFunctionDecls(SEM::Namespace* nameSpace) {
					const std::vector<SEM::Namespace*>& namespaces = nameSpace->namespaces();
					
					for (size_t i = 0; i < namespaces.size(); i++) {
						genNamespaceFunctionDecls(namespaces.at(i));
					}
					
					const std::vector<SEM::TypeInstance*>& typeInstances = nameSpace->typeInstances();
					
					for (size_t i = 0; i < typeInstances.size(); i++) {
						genTypeInstanceFunctionDecls(typeInstances.at(i));
					}
					
					const std::vector<SEM::Function*>& functions = nameSpace->functions();
					
					for (size_t i = 0; i < functions.size(); i++) {
						genFunctionDecl(NULL, functions.at(i));
					}
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
				
				void genFunctionDef(SEM::Function* function, SEM::TypeInstance* typeInstance) {
					assert(function != NULL && "Generating a function definition requires a non-NULL SEM Function object");
					
					if (function->isDeclaration()) {
						return;
					}
					
					if (typeInstance != NULL) {
						LOG(LOG_NOTICE, "Generating method definition for '%s' in type '%s'.",
							function->name().toString().c_str(), typeInstance->name().toString().c_str());
					} else {
						LOG(LOG_NOTICE, "Generating function definition for '%s'.",
							function->name().toString().c_str());
					}
					
					llvm::Function* llvmFunction = module_.getFunctionMap().get(function);
					
					llvmFunction->dump();
					
					Function genFunction(module_, *llvmFunction, getArgInfo(function));
					
					// Parameters need to be copied to the stack, so that it's
					// possible to assign to them, take their address, etc.
					const std::vector<SEM::Var*>& parameterVars = function->parameters();
					
					for (std::size_t i = 0; i < parameterVars.size(); i++) {
						SEM::Var* paramVar = parameterVars.at(i);
						assert(paramVar->kind() == SEM::Var::PARAM);
						
						// Create an alloca for this variable.
						llvm::Value* stackObject = genAlloca(genFunction, paramVar->type());
						
						// Store the initial value into the alloca.
						genStore(genFunction, genFunction.getArg(i), stackObject, paramVar->type());
						
						genFunction.getLocalVarMap().insert(paramVar, stackObject);
					}
					
					genScope(genFunction, function->scope());
					
					// Need to terminate the final basic block.
					// (just make it loop to itself - this will
					// be removed by dead code elimination)
					genFunction.getBuilder().CreateBr(genFunction.getSelectedBasicBlock());
					
					llvmFunction->dump();
					
					// Check the generated function is correct.
					genFunction.verify();
				}
				
				void genTypeInstanceFunctionDefs(SEM::TypeInstance* typeInstance) {
					if (typeInstance->isClass()) {
						const std::vector<SEM::Function*>& functions = typeInstance->functions();
						
						for (size_t i = 0; i < functions.size(); i++) {
							genFunctionDef(functions.at(i), typeInstance);
						}
					} else if (typeInstance->isPrimitive()) {
						const std::vector<SEM::Function*>& functions = typeInstance->functions();
						
						for (size_t i = 0; i < functions.size(); i++) {
							SEM::Function* function = functions.at(i);
							createPrimitiveMethod(module_, typeInstance->name().last(),
								function->name().last(), *(module_.getFunctionMap().get(function)));
						}
					}
				}
				
				void genNamespaceFunctionDefs(SEM::Namespace* nameSpace) {
					const std::vector<SEM::Namespace*>& namespaces = nameSpace->namespaces();
					
					for (size_t i = 0; i < namespaces.size(); i++) {
						genNamespaceFunctionDefs(namespaces.at(i));
					}
					
					const std::vector<SEM::TypeInstance*>& typeInstances = nameSpace->typeInstances();
					
					for (size_t i = 0; i < typeInstances.size(); i++) {
						genTypeInstanceFunctionDefs(typeInstances.at(i));
					}
					
					const std::vector<SEM::Function*>& functions = nameSpace->functions();
					
					for (size_t i = 0; i < functions.size(); i++) {
						genFunctionDef(functions.at(i), NULL);
					}
				}
				
		};
		
		CodeGenerator::CodeGenerator(const TargetInfo& targetInfo, const std::string& moduleName) {
			codeGen_ = new InternalCodeGen(moduleName, targetInfo);
		}
		
		CodeGenerator::~CodeGenerator() {
			delete codeGen_;
		}
		
		void CodeGenerator::applyOptimisations(size_t optLevel) {
			Optimisations optimisations(codeGen_->getModule());
			optimisations.addDefaultPasses(optLevel);
			optimisations.run();
		}
		
		void CodeGenerator::genNamespace(SEM::Namespace* nameSpace) {
			codeGen_->genNamespaceTypePlaceholders(nameSpace);
			codeGen_->genNamespaceTypeMembers(nameSpace);
			codeGen_->genNamespaceFunctionDecls(nameSpace);
			codeGen_->genNamespaceFunctionDefs(nameSpace);
		}
		
		void CodeGenerator::writeToFile(const std::string& fileName) {
			codeGen_->getModule().writeBitCodeToFile(fileName);
		}
		
		void CodeGenerator::dumpToFile(const std::string& fileName) {
			codeGen_->getModule().dumpToFile(fileName);
		}
		
		void CodeGenerator::dump() {
			codeGen_->getModule().dump();
		}
		
	}
	
}

