#include <llvm/Attributes.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/DerivedTypes.h>
#include <llvm/InlineAsm.h>
#include <llvm/IRBuilder.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_os_ostream.h>

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
#include <Locic/CodeGen/Mangling.hpp>
#include <Locic/CodeGen/Primitives.hpp>
#include <Locic/CodeGen/Support.hpp>
#include <Locic/CodeGen/TargetInfo.hpp>
#include <Locic/CodeGen/VTable.hpp>

namespace Locic {

	namespace CodeGen {
	
		Map<MethodHash, SEM::Function*> CreateFunctionHashMap(SEM::TypeInstance* typeInstance) {
			Map<MethodHash, SEM::Function*> hashMap;
			
			const std::vector<SEM::Function*>& functions = typeInstance->functions();
			
			for (size_t i = 0; i < functions.size(); i++) {
				SEM::Function* function = functions.at(i);
				hashMap.insert(CreateMethodNameHash(function->name().last()), function);
			}
			
			return hashMap;
		}
		
		std::vector<MethodHash> CreateHashArray(const Map<MethodHash, SEM::Function*>& hashMap) {
			std::vector<MethodHash> hashArray;
			
			Map<MethodHash, SEM::Function*>::Range range = hashMap.range();
			
			for (; !range.empty(); range.popFront()) {
				hashArray.push_back(range.front().key());
			}
			
			assert(hashMap.size() == hashArray.size());
			
			return hashArray;
		}
		
		class InternalCodeGen {
			private:
				std::string name_;
				TargetInfo targetInfo_;
				llvm::Module* module_;
				llvm::IRBuilder<> builder_;
				llvm::FunctionType* currentFunctionType_;
				llvm::Function* currentFunction_;
				llvm::BasicBlock* currentBasicBlock_;
				
				Map<SEM::TypeInstance*, llvm::StructType*> typeInstances_;
				
				Map<SEM::Function*, llvm::Function*> functions_;
				Map<SEM::Var*, size_t> memberVarOffsets_;
				Map<SEM::Var*, llvm::Value*> localVariables_, paramVariables_;
				llvm::Value* returnVar_;
				llvm::Value* thisPointer_;
				llvm::StructType* typenameType_;
				
			public:
				InternalCodeGen(const TargetInfo& targetInfo, const std::string& moduleName)
					: name_(moduleName),
					  targetInfo_(targetInfo),
					  module_(new llvm::Module(name_.c_str(), llvm::getGlobalContext())),
					  builder_(llvm::getGlobalContext()),
					  returnVar_(NULL),
					  thisPointer_(NULL) {
					module_->setTargetTriple(targetInfo_.getTargetTriple());
					
					std::vector<llvm::Type*> structElementTypes;
					structElementTypes.push_back(getVTableType(targetInfo_)->getPointerTo());
					structElementTypes.push_back(i8PtrType());
					typenameType_ = llvm::StructType::create(llvm::getGlobalContext(),
									structElementTypes, "typename");
				}
				
				~InternalCodeGen() {
					delete module_;
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
					
					llvm::StructType* structType = llvm::StructType::create(llvm::getGlobalContext(),
												   mangleTypeName(typeInstance->name()));
												   
					typeInstances_.insert(typeInstance, structType);
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
					llvm::StructType* structType = typeInstances_.get(typeInstance);
					
					if (typeInstance->isClassDef() || typeInstance->isStructDef()) {
						// Generating the type for a class or struct definition, so
						// the size and contents of the type instance is known and
						// hence the contents can be specified.
						std::vector<llvm::Type*> structVariables;
						
						// Pointer for template context information.
						structVariables.push_back(i8PtrType());
						
						// Add member variables.
						const std::vector<SEM::Var*>& variables = typeInstance->variables();
						
						for (size_t i = 0; i < variables.size(); i++) {
							SEM::Var* var = variables.at(i);
							structVariables.push_back(genType(var->type()));
							memberVarOffsets_.insert(var, 1 + i);
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
						
					const size_t parentNumTemplateArgs =
						parent != NULL ?
						parent->templateVariables().size() :
						0;
						
					llvm::Type* contextPtrType =
						function->isMethod() && !function->isStatic() ?
						getTypeInstancePointer(parent) :
						NULL;
						
					llvm::Function* functionDecl = llvm::Function::Create(genFunctionType(function->type(),
												   contextPtrType), getFunctionLinkage(parent),
												   mangleFunctionName(function->name()), module_);
												   
					if (function->type()->getFunctionReturnType()->isClassOrTemplateVar()) {
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
						
						functionDecl->addAttribute(1,
												   llvm::Attributes::get(llvm::getGlobalContext(),
														   attributes));
					}
					
					functions_.insert(function, functionDecl);
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
				
				void genInterfaceMethod(SEM::Function* function) {
					assert(function->isMethod());
					
					if (function->isStatic()) {
						// Don't process static methods of interfaces.
						return;
					}
					
					assert(function->isDeclaration() && "Interface methods must be declarations");
					
					LOG(LOG_INFO, "Generating interface method '%s'.",
						function->name().toString().c_str());
						
					llvm::Function* generatedFunction = functions_.get(function);
					assert(generatedFunction != NULL);
					
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", generatedFunction);
					builder_.SetInsertPoint(basicBlock);
					
					// Store arguments onto stack.
					llvm::Function::arg_iterator arg = generatedFunction->arg_begin();
					SEM::Type* returnType = function->type()->getFunctionReturnType();
					llvm::Value* returnVar = returnType->isClassOrTemplateVar() ? arg++ : NULL;
					
					// Get the 'this' record, which is the
					// pair of the 'this' pointer and the
					// method vtable pointer.
					llvm::Value* thisRecord = arg++;
					
					// Get the 'this' pointer.
					llvm::Value* thisPointer = builder_.CreateExtractValue(thisRecord, std::vector<unsigned>(1, 0), "thisPointer");
					
					// Get the vtable pointer.
					llvm::Value* vtablePointer = builder_.CreateExtractValue(thisRecord,
												 std::vector<unsigned>(1, 1), "vtablePointer");
												 
					const MethodHash methodHash = CreateMethodNameHash(function->name().last());
					const size_t offset = methodHash % VTABLE_SIZE;
					
					std::vector<llvm::Value*> vtableEntryGEP;
					vtableEntryGEP.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, 0)));
					vtableEntryGEP.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, 2)));
					vtableEntryGEP.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, offset)));
					
					llvm::Value* vtableEntryPointer = builder_.CreateInBoundsGEP(vtablePointer, vtableEntryGEP, "vtableEntryPointer");
					llvm::Value* methodFunctionPointer = builder_.CreateLoad(vtableEntryPointer, "methodFunctionPointer");
					llvm::Type* methodFunctionType = genFunctionType(function->type(), thisPointer->getType());
					llvm::Value* castedMethodFunctionPointer = builder_.CreatePointerCast(
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
					builder_.CreateCall(setEax);
					
					const bool isVoidReturnType = returnType->isVoid() || returnType->isClassOrTemplateVar();
					
					llvm::Value* methodCallValue = builder_.CreateCall(castedMethodFunctionPointer,
												   arguments, isVoidReturnType ? "" : "methodCallValue");
												   
					if (isVoidReturnType) {
						builder_.CreateRetVoid();
					} else {
						builder_.CreateRet(methodCallValue);
					}
					
					// Check the generated function is correct.
					verifyFunction(*generatedFunction);
				}
				
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
					
					currentFunction_ = functions_.get(function);
					assert(currentFunction_ != NULL);
					
					currentFunction_->dump();
					
					currentBasicBlock_ = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", currentFunction_);
					
					builder_.SetInsertPoint(currentBasicBlock_);
					
					// Store arguments onto stack.
					llvm::Function::arg_iterator arg = currentFunction_->arg_begin();
					SEM::Type* returnType = function->type()->getFunctionReturnType();
					
					if (returnType->isClassOrTemplateVar()) {
						returnVar_ = arg++;
					} else {
						returnVar_ = NULL;
					}
					
					if (function->isMethod() && !function->isStatic()) {
						assert(typeInstance != NULL);
						// Generating a dynamic method, so capture the 'this' pointer.
						thisPointer_ = arg++;
					} else {
						thisPointer_ = NULL;
					}
					
					// Get template variable vtables, if this is a method.
					if (function->isMethod()) {
						if (function->isStatic()) {
							// For static methods, they are passed as arguments.
							llvm::Value* typenameArray = arg++;
							assert(typenameArray != NULL);
							
							for (size_t i = 0; i < typeInstance->templateVariables().size(); i++) {
								llvm::Value* typenameValue = builder_.CreateLoad(
																 builder_.CreateConstGEP2_32(typenameArray, 0, i));
								templateVarValues_.insert(typeInstance->templateVariables().at(i),
														  typenameValue);
							}
						} else {
							assert(thisPointer_ != NULL);
							
							// For normal methods, they are stored in the parent type.
							llvm::Value* typenameArray = builder_.CreateLoad(
															 builder_.CreateConstGEP2_32(thisPointer_, 0, 0));
															 
							for (size_t i = 0; i < typeInstance->templateVariables().size(); i++) {
								llvm::Value* typenameValue = builder_.CreateLoad(
																 builder_.CreateConstGEP2_32(typenameArray, 0, i));
								templateVarValues_.insert(typeInstance->templateVariables().at(i),
														  typenameValue);
							}
						}
					}
					
					assert(paramVariables_.empty());
					assert(localVariables_.empty());
					
					const std::vector<SEM::Var*>& parameterVars = function->parameters();
					
					for (std::size_t i = 0; i < parameterVars.size(); ++arg, i++) {
						assert(arg != currentFunction_->arg_end());
						
						SEM::Var* paramVar = parameterVars.at(i);
						
						assert(paramVar->kind() == SEM::Var::PARAM);
						
						// Create an alloca for this variable.
						llvm::Value* stackObject = genAlloca(paramVar->type());
						
						paramVariables_.insert(paramVar, stackObject);
						
						// Store the initial value into the alloca.
						genStore(arg, stackObject, paramVar->type());
					}
					
					genScope(function->scope());
					
					// Need to terminate the final basic block.
					// (just make it loop to itself - this will
					// be removed by dead code elimination)
					builder_.CreateBr(builder_.GetInsertBlock());
					
					currentFunction_->dump();
					
					// Check the generated function is correct.
					verifyFunction(*currentFunction_);
					
					templateVarValues_.clear();
					paramVariables_.clear();
					localVariables_.clear();
					returnVar_ = NULL;
					thisPointer_ = NULL;
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
							createPrimitiveMethod(*module_, typeInstance->name().last(),
												  function->name().last(), functions_.get(function));
						}
					} else if (typeInstance->isInterface()) {
						const std::vector<SEM::Function*>& functions = typeInstance->functions();
						
						for (size_t i = 0; i < functions.size(); i++) {
							genInterfaceMethod(functions.at(i));
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
				
				// ---- Function code generation.
				void genScope(const SEM::Scope& scope) {
					for (std::size_t i = 0; i < scope.localVariables().size(); i++) {
						SEM::Var* localVar = scope.localVariables().at(i);
						
						// Create an alloca for this variable.
						llvm::Value* stackObject = genAlloca(localVar->type());
						
						localVariables_.forceInsert(localVar, stackObject);
					}
					
					for (std::size_t i = 0; i < scope.statements().size(); i++) {
						genStatement(scope.statements().at(i));
					}
				}
				
				void genStatement(SEM::Statement* statement) {
					switch (statement->kind()) {
						case SEM::Statement::VALUE: {
							genValue(statement->getValue());
							break;
						}
						
						case SEM::Statement::SCOPE: {
							genScope(statement->getScope());
							break;
						}
						
						case SEM::Statement::IF: {
							llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(llvm::getGlobalContext(),
													   "then", currentFunction_);
							llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(llvm::getGlobalContext(),
													   "else");
							llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(llvm::getGlobalContext(),
														"ifmerge");
							builder_.CreateCondBr(genValue(statement->getIfCondition()), thenBB, elseBB);
							// Create 'then'.
							builder_.SetInsertPoint(thenBB);
							genScope(statement->getIfTrueScope());
							builder_.CreateBr(mergeBB);
							// Create 'else'.
							currentFunction_->getBasicBlockList().push_back(elseBB);
							builder_.SetInsertPoint(elseBB);
							
							if (statement->hasIfFalseScope()) {
								genScope(statement->getIfFalseScope());
							}
							
							builder_.CreateBr(mergeBB);
							// Create merge.
							currentFunction_->getBasicBlockList().push_back(mergeBB);
							builder_.SetInsertPoint(mergeBB);
							break;
						}
						
						case SEM::Statement::WHILE: {
							llvm::BasicBlock* insideLoopBB =
								llvm::BasicBlock::Create(llvm::getGlobalContext(), "insideLoop", currentFunction_);
							llvm::BasicBlock* afterLoopBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "afterLoop");
							builder_.CreateCondBr(genValue(statement->getWhileCondition()), insideLoopBB, afterLoopBB);
							// Create loop contents.
							builder_.SetInsertPoint(insideLoopBB);
							genScope(statement->getWhileScope());
							builder_.CreateCondBr(genValue(statement->getWhileCondition()), insideLoopBB, afterLoopBB);
							// Create 'else'.
							currentFunction_->getBasicBlockList().push_back(afterLoopBB);
							builder_.SetInsertPoint(afterLoopBB);
							break;
						}
						
						case SEM::Statement::ASSIGN: {
							SEM::Value* lValue = statement->getAssignLValue();
							SEM::Value* rValue = statement->getAssignRValue();
							genStore(genValue(rValue), genValue(lValue, true), rValue->type());
							break;
						}
						
						case SEM::Statement::RETURN: {
							if (statement->getReturnValue() != NULL
								&& !statement->getReturnValue()->type()->isVoid()) {
								llvm::Value* returnValue = genValue(statement->getReturnValue());
								
								if (returnVar_ != NULL) {
									genStore(returnValue, returnVar_, statement->getReturnValue()->type());
									builder_.CreateRetVoid();
								} else {
									builder_.CreateRet(returnValue);
								}
							} else {
								builder_.CreateRetVoid();
							}
							
							// Need a basic block after a return statement in case anything more is generated.
							// This (and any following code) will be removed by dead code elimination.
							builder_.SetInsertPoint(llvm::BasicBlock::Create(llvm::getGlobalContext(),
													"next", currentFunction_));
							break;
						}
						
						default:
							assert(false && "Unknown statement type");
							break;
					}
				}
				
				llvm::Value* generateLValue(SEM::Value* value) {
					if (value->type()->isLValue()) {
						return genValue(value, true);
					} else {
						llvm::Value* lValue = genAlloca(value->type());
						llvm::Value* rValue = genValue(value);
						genStore(rValue, lValue, value->type());
						return lValue;
					}
				}
				
				llvm::Function* createSubstitutionStub(llvm::Function* function, SEM::Type* sourceType, SEM::Type* destType) {
					assert(sourceType->isFunction());
					assert(destType->isFunction());
					
					SEM::Type* sourceReturnType = sourceType->getFunctionReturnType();
					SEM::Type* destReturnType = destType->getFunctionReturnType();
					
					// TODO: A whole load of things need to be done here, but
					//       for now this only converts template var return value
					//       to a primitive return value.
					if (sourceReturnType->isTemplateVar() && !destReturnType->isClassOrTemplateVar()) {
						assert(function->arg_size() >= 1);
						
						// Extract parent type if there is one.
						llvm::Type* parent = (function->getArgumentList().size() >
											  (sourceType->getFunctionParameterTypes().size() + 1))
											 ? (++(function->getArgumentList().begin()))->getType() : NULL;
											 
						llvm::Function* stub = llvm::Function::Create(genFunctionType(destType, parent),
											   llvm::Function::InternalLinkage, "", module_);
						assert((stub->arg_size() + 1) == function->arg_size());
						
						llvm::IRBuilder<> builder(module_->getContext());
						
						llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(module_->getContext(), "entry", stub);
						builder.SetInsertPoint(basicBlock);
						
						LOG(LOG_INFO, "Creating substitution stub from type %s to %s.",
							sourceType->toString().c_str(), destType->toString().c_str());
							
						llvm::Value* captureVar = builder.CreateAlloca(genType(destReturnType));
						std::vector<llvm::Value*> arguments;
						arguments.push_back(builder.CreatePointerCast(captureVar, genType(sourceReturnType)->getPointerTo()));
						
						for (llvm::Function::arg_iterator arg = stub->arg_begin(); arg != stub->arg_end(); ++arg) {
							arguments.push_back(arg);
						}
						
						llvm::Value* returnValue = builder.CreateCall(function, arguments);
						assert(returnValue->getType()->isVoidTy());
						
						builder.CreateRet(builder.CreateLoad(captureVar));
						
						return stub;
					} else {
						return function;
					}
				}
				
				llvm::Value* genValue(SEM::Value* value, bool genLValue = false) {
					assert(value != NULL && "Cannot generate NULL value");
					
					LOG(LOG_INFO, "Generating value %s.",
						value->toString().c_str());
						
					switch (value->kind()) {
						case SEM::Value::CONSTANT: {
							switch (value->constant->getType()) {
								case Locic::Constant::NULLVAL:
									return llvm::ConstantPointerNull::get(
											   llvm::PointerType::getUnqual(
												   llvm::Type::getInt8Ty(llvm::getGlobalContext())));
												   
								case Locic::Constant::BOOLEAN:
									return llvm::ConstantInt::get(llvm::getGlobalContext(),
																  llvm::APInt(1, value->constant->getBool()));
																  
								case Locic::Constant::SIGNEDINT: {
									const std::size_t primitiveSize = targetInfo_.getPrimitiveSize(
																		  value->constant->getTypeName());
									return llvm::ConstantInt::get(llvm::getGlobalContext(),
																  llvm::APInt(primitiveSize, value->constant->getInt()));
								}
								
								case Locic::Constant::UNSIGNEDINT: {
									const std::size_t primitiveSize = targetInfo_.getPrimitiveSize(
																		  value->constant->getTypeName());
									return llvm::ConstantInt::get(llvm::getGlobalContext(),
																  llvm::APInt(primitiveSize, value->constant->getUint()));
								}
								
								case Locic::Constant::FLOATINGPOINT: {
									switch (value->constant->getFloatType()) {
										case Locic::Constant::FLOAT:
											return llvm::ConstantFP::get(llvm::getGlobalContext(),
																		 llvm::APFloat((float) value->constant->getFloat()));
																		 
										case Locic::Constant::DOUBLE:
											return llvm::ConstantFP::get(llvm::getGlobalContext(),
																		 llvm::APFloat((double) value->constant->getFloat()));
																		 
										case Locic::Constant::LONGDOUBLE:
											assert(false && "Long double not implemented yet");
											return NULL;
											
										default:
											assert(false && "Unknown float constant type");
											return NULL;
									}
								}
								
								case Locic::Constant::STRING: {
									const std::string stringValue = value->constant->getString();
									
									switch (value->constant->getStringType()) {
										case Locic::Constant::CSTRING: {
											const bool isConstant = true;
											llvm::ArrayType* arrayType = llvm::ArrayType::get(
																			 llvm::Type::getInt8Ty(llvm::getGlobalContext()),
																			 stringValue.size() + 1);
											llvm::Constant* constArray =
												llvm::ConstantDataArray::getString(
													llvm::getGlobalContext(), stringValue.c_str());
											llvm::GlobalVariable* globalArray = new llvm::GlobalVariable(
												*module_, arrayType, isConstant,
												llvm::GlobalValue::PrivateLinkage,
												constArray, "");
											globalArray->setAlignment(1);
											// Convert array to a pointer.
											return builder_.CreateConstGEP2_32(globalArray, 0, 0);
										}
										
										case Locic::Constant::LOCISTRING: {
											assert(false && "Loci string constants not yet implemented");
											return NULL;
										}
										
										default:
											assert(false && "Unknown string constant type");
											return NULL;
									}
								}
								
								default:
									assert(false && "Unknown constant type");
									return llvm::UndefValue::get(llvm::Type::getVoidTy(llvm::getGlobalContext()));
							}
						}
						
						case SEM::Value::COPY: {
							// TODO!
							return genValue(value->copyValue.value);
						}
						
						case SEM::Value::VAR: {
							SEM::Var* var = value->varValue.var;
							
							switch (var->kind()) {
								case SEM::Var::PARAM: {
									llvm::Value* val = paramVariables_.get(var);
									
									if (genLValue) {
										return val;
									} else {
										return genLoad(val, value->type());
									}
								}
								
								case SEM::Var::LOCAL: {
									llvm::Value* val = localVariables_.get(var);
									
									if (genLValue) {
										return val;
									} else {
										return genLoad(val, value->type());
									}
								}
								
								case SEM::Var::MEMBER: {
									assert(thisPointer_ != NULL &&
										   "The 'this' pointer cannot be null when accessing member variables");
									llvm::Value* memberPtr = builder_.CreateConstInBoundsGEP2_32(
																 thisPointer_, 0, memberVarOffsets_.get(var));
																 
									if (genLValue) {
										return memberPtr;
									} else {
										return genLoad(memberPtr, value->type());
									}
								}
								
								default: {
									assert(false && "Unknown variable type in variable access");
									return llvm::UndefValue::get(llvm::Type::getVoidTy(llvm::getGlobalContext()));
								}
							}
						}
						
						case SEM::Value::ADDRESSOF: {
							return genValue(value->addressOf.value, true);
						}
						
						case SEM::Value::DEREF_POINTER: {
							if (genLValue) {
								return genValue(value->derefPointer.value);
							} else {
								return genLoad(genValue(value->derefPointer.value), value->type());
							}
						}
						
						case SEM::Value::REFERENCEOF: {
							return genValue(value->referenceOf.value, true);
						}
						
						case SEM::Value::DEREF_REFERENCE: {
							if (genLValue) {
								return genValue(value->derefReference.value);
							} else {
								return genLoad(genValue(value->derefReference.value), value->type());
							}
						}
						
						case SEM::Value::TERNARY: {
							return builder_.CreateSelect(genValue(value->ternary.condition),
														 genValue(value->ternary.ifTrue, genLValue),
														 genValue(value->ternary.ifFalse, genLValue));
						}
						
						case SEM::Value::CAST: {
							llvm::Value* codeValue = genValue(value->cast.value, genLValue);
							SEM::Type* sourceType = value->cast.value->type();
							SEM::Type* destType = value->type();
							assert((sourceType->kind() == destType->kind()
									|| sourceType->isNull()
									|| destType->isVoid())
								   && "Types must be in the same group for cast, or "
								   "it should be a cast from null, or a cast to void");
								   
							LOG(LOG_NOTICE, "Generating cast from type %s to type %s.",
								sourceType->toString().c_str(), destType->toString().c_str());
								
							if (destType->isVoid()) {
								// All casts to void have the same outcome.
								return llvm::UndefValue::get(llvm::Type::getVoidTy(llvm::getGlobalContext()));
							}
							
							switch (sourceType->kind()) {
								case SEM::Type::VOID: {
									return codeValue;
								}
								
								case SEM::Type::NULLT: {
									switch (destType->kind()) {
										case SEM::Type::NULLT:
											return codeValue;
											
										case SEM::Type::POINTER:
										case SEM::Type::FUNCTION:
											return builder_.CreatePointerCast(codeValue, genType(destType));
											
										case SEM::Type::OBJECT: {
											assert(false && "TODO");
											return NULL;
										}
										
										default: {
											assert(false && "Invalid cast from null");
											return llvm::UndefValue::get(llvm::Type::getVoidTy(
																			 llvm::getGlobalContext()));
										}
									}
								}
								
								case SEM::Type::OBJECT: {
									if (sourceType->getObjectType() == destType->getObjectType()) {
										return codeValue;
									}
									
									assert(false && "Casts between named types not implemented");
									return NULL;
								}
								
								case SEM::Type::REFERENCE: {
									if (genLValue) {
										return builder_.CreatePointerCast(codeValue,
																		  genType(destType)->getPointerTo());
									} else {
										return builder_.CreatePointerCast(codeValue, genType(destType));
									}
								}
								
								case SEM::Type::POINTER: {
									if (genLValue) {
										return builder_.CreatePointerCast(codeValue,
																		  genType(destType)->getPointerTo());
									} else {
										return builder_.CreatePointerCast(codeValue, genType(destType));
									}
								}
								
								case SEM::Type::FUNCTION: {
									return codeValue;
								}
								
								case SEM::Type::METHOD: {
									return codeValue;
								}
								
								case SEM::Type::TEMPLATEVAR: {
									return codeValue;
								}
								
								default:
									assert(false && "Unknown type in cast");
									return llvm::UndefValue::get(llvm::Type::getVoidTy(llvm::getGlobalContext()));
							}
						}
						
						case SEM::Value::POLYCAST: {
							assert(!genLValue && "Cannot generate interfaces as lvalues in polycast");
							llvm::Value* rawValue = genValue(value->polyCast.value);
							SEM::Type* sourceType = value->polyCast.value->type();
							SEM::Type* destType = value->type();
							assert((sourceType->isPointer() || sourceType->isReference())  && "Polycast source type must be pointer or reference.");
							assert((destType->isPointer() || destType->isReference()) && "Polycast dest type must be pointer or reference.");
							SEM::Type* sourceTarget = sourceType->getPointerOrReferenceTarget();
							SEM::Type* destTarget = destType->getPointerOrReferenceTarget();
							assert(destTarget->isInterface() && "Polycast dest target type must be interface");
							
							if (sourceTarget->isInterface()) {
								// Get the object pointer.
								llvm::Value* objectPointerValue = builder_.CreateExtractValue(rawValue,
																  std::vector<unsigned>(1, 0));
																  
								// Cast it as a pointer to the opaque struct representing
								// destination interface type.
								llvm::Value* objectPointer = builder_.CreatePointerCast(objectPointerValue,
															 typeInstances_.get(destTarget->getObjectType())->getPointerTo());
															 
								// Get the vtable pointer.
								llvm::Value* vtablePointer = builder_.CreateExtractValue(rawValue,
															 std::vector<unsigned>(1, 1));
															 
								// Build the new interface pointer struct with these values.
								llvm::Value* interfaceValue = llvm::UndefValue::get(genType(destType));
								interfaceValue = builder_.CreateInsertValue(interfaceValue, objectPointer,
												 std::vector<unsigned>(1, 0));
								interfaceValue = builder_.CreateInsertValue(interfaceValue, vtablePointer,
												 std::vector<unsigned>(1, 1));
								return interfaceValue;
							} else {
								// Cast class pointer to pointer to the opaque struct
								// representing destination interface type.
								llvm::Value* objectPointer = builder_.CreatePointerCast(rawValue,
															 typeInstances_.get(destTarget->getObjectType())->getPointerTo());
															 
								// Create the vtable.
								llvm::Value* vtablePointer = genVTable(sourceTarget);
								
								// Build the new interface pointer struct with these values.
								llvm::Value* interfaceValue = llvm::UndefValue::get(genType(destType));
								interfaceValue = builder_.CreateInsertValue(interfaceValue, objectPointer,
												 std::vector<unsigned>(1, 0));
								interfaceValue = builder_.CreateInsertValue(interfaceValue, vtablePointer,
												 std::vector<unsigned>(1, 1));
								return interfaceValue;
							}
						}
						
						case SEM::Value::INTERNALCONSTRUCT: {
							const std::vector<SEM::Value*>& parameters = value->internalConstruct.parameters;
							llvm::Value* objectValue = genAlloca(value->type());
							
							LOG(LOG_INFO, "Type is %s.",
								value->type()->toString().c_str());
								
							genType(value->type())->dump();
							
							objectValue->dump();
							
							// TODO: need to get the actual template parameter values,
							//        NOT pass a NULL pointer!
							llvm::Value* contextPointer = llvm::ConstantPointerNull::get(
															  llvm::PointerType::getUnqual(
																  llvm::Type::getInt8Ty(llvm::getGlobalContext())));
																  
							builder_.CreateStore(contextPointer,
												 builder_.CreateConstInBoundsGEP2_32(objectValue, 0, 0),
												 "store_context_ptr");
												 
							for (size_t i = 0; i < parameters.size(); i++) {
								SEM::Value* paramValue = parameters.at(i);
								genStore(genValue(paramValue),
										 builder_.CreateConstInBoundsGEP2_32(objectValue, 0, i + 1),
										 paramValue->type());
							}
							
							return objectValue;
						}
						
						case SEM::Value::MEMBERACCESS: {
							const size_t offset = memberVarOffsets_.get(value->memberAccess.memberVar);
							
							if (genLValue) {
								return builder_.CreateConstInBoundsGEP2_32(
										   genValue(value->memberAccess.object, true), 0,
										   offset);
							} else {
								return builder_.CreateExtractValue(
										   genValue(value->memberAccess.object),
										   std::vector<unsigned>(1, offset));
							}
						}
						
						case SEM::Value::FUNCTIONCALL: {
							LOG(LOG_EXCESSIVE, "Generating function call value %s.",
								value->functionCall.functionValue->toString().c_str());
								
							llvm::Value* function = genValue(value->functionCall.functionValue);
							assert(function->getType()->isPointerTy());
							llvm::Type* functionType = function->getType()->getPointerElementType();
							assert(functionType->isFunctionTy());
							std::vector<llvm::Value*> parameters;
							const std::vector<SEM::Value*>& paramList = value->functionCall.parameters;
							SEM::Type* returnType = value->type();
							llvm::Value* returnValue = NULL;
							
							if (returnType->isClassOrTemplateVar()) {
								returnValue = genAlloca(returnType);
								assert(returnValue != NULL &&
									   "Must have lvalue for holding class "
									   "return value so it can be passed by reference");
								parameters.push_back(returnValue);
							}
							
							LOG(LOG_NOTICE, "Function:");
							function->dump();
							
							for (std::size_t i = 0; i < paramList.size(); i++) {
								llvm::Value* argValue = genValue(paramList.at(i));
								
								// When calling var-args functions, all 'char' and
								// 'short' values must be extended to 'int' values,
								// and all 'float' values must be converted to 'double'
								// values.
								if (functionType->isFunctionVarArg()) {
									llvm::Type* argType = argValue->getType();
									const unsigned sizeInBits = argType->getPrimitiveSizeInBits();
									
									if (argType->isIntegerTy() && sizeInBits < targetInfo_.getPrimitiveSize("int")) {
										// Need to extend to int.
										// TODO: this doesn't handle unsigned types; perhaps
										// this code should be moved to semantic analysis.
										argValue = builder_.CreateSExt(argValue,
																	   llvm::IntegerType::get(llvm::getGlobalContext(),
																			   targetInfo_.getPrimitiveSize("int")));
									} else if (argType->isFloatingPointTy() && sizeInBits < 64) {
										// Need to extend to double.
										argValue = builder_.CreateFPExt(argValue,
																		llvm::Type::getDoubleTy(llvm::getGlobalContext()));
									}
								}
								
								parameters.push_back(argValue);
								
								LOG(LOG_NOTICE, "    Param %llu:",
									(unsigned long long) i);
								argValue->dump();
							}
							
							const size_t numFunctionArgs =
								llvm::cast<llvm::Function>(function)->arg_size();
								
							if (numFunctionArgs != parameters.size()) {
								LOG(LOG_NOTICE, "POSSIBLE ERROR: number of arguments given (%llu) "
									"doesn't match required number (%llu).",
									(unsigned long long) parameters.size(),
									(unsigned long long) numFunctionArgs);
							}
							
							//assert(numFunctionArgs == parameters.size());
							
							llvm::Value* callReturnValue = builder_.CreateCall(function, parameters);
							
							if (returnValue != NULL) {
								return genLoad(returnValue, returnType);
							} else {
								return callReturnValue;
							}
						}
						
						case SEM::Value::FUNCTIONREF: {
							SEM::Function* semFunction = value->functionRef.function;
							llvm::Function* function = functions_.get(semFunction);
							assert(function != NULL && "FunctionRef requires a valid function");
							return createSubstitutionStub(function, semFunction->type(), value->type());
						}
						
						case SEM::Value::STATICMETHODREF: {
							SEM::Function* semFunction = value->staticMethodRef.function;
							llvm::Function* function = functions_.get(semFunction);
							assert(function != NULL && "StaticMethodRef requires a valid function");
							
							llvm::Value* methodValue = llvm::UndefValue::get(genType(value->type()));
							
							llvm::Value* functionPtr =
								builder_.CreatePointerCast(function,
														   genFunctionType(value->type()->getMethodFunctionType(), i8PtrType())->getPointerTo(),
														   "static_method_function_ptr");
														   
							// TODO: need to generate the actual template parameter values,
							//        NOT pass a NULL pointer!
							llvm::Value* contextPointer = llvm::ConstantPointerNull::get(
															  llvm::PointerType::getUnqual(
																  llvm::Type::getInt8Ty(llvm::getGlobalContext())));
																  
							methodValue = builder_.CreateInsertValue(methodValue, functionPtr, std::vector<unsigned>(1, 0));
							methodValue = builder_.CreateInsertValue(methodValue, contextPointer, std::vector<unsigned>(1, 1));
							
							return methodValue;
						}
						
						case SEM::Value::METHODOBJECT: {
							llvm::Value* function = genValue(value->methodObject.method);
							assert(function != NULL && "MethodObject requires a valid function");
							llvm::Value* dataPointer = generateLValue(value->methodObject.methodOwner);
							assert(dataPointer != NULL && "MethodObject requires a valid data pointer");
							
							assert(value->type()->isMethod());
							
							llvm::Value* methodValue = llvm::UndefValue::get(genType(value->type()));
							
							function->dump();
							dataPointer->dump();
							methodValue->dump();
							
							llvm::Value* functionPtr =
								builder_.CreatePointerCast(function,
														   genFunctionType(value->type()->getMethodFunctionType(), i8PtrType())->getPointerTo(),
														   "dynamic_method_function_ptr");
														   
							llvm::Value* contextPtr =
								builder_.CreatePointerCast(dataPointer, i8PtrType(), "this_ptr_cast_to_void_ptr");
								
							methodValue = builder_.CreateInsertValue(methodValue, functionPtr, std::vector<unsigned>(1, 0));
							methodValue = builder_.CreateInsertValue(methodValue, contextPtr, std::vector<unsigned>(1, 1));
							return methodValue;
						}
						
						case SEM::Value::METHODCALL: {
							LOG(LOG_EXCESSIVE, "Generating method call value %s.",
								value->methodCall.methodValue->toString().c_str());
								
							llvm::Value* method = genValue(value->methodCall.methodValue);
							llvm::Value* function = builder_.CreateExtractValue(method, std::vector<unsigned>(1, 0));
							llvm::Value* contextPointer = builder_.CreateExtractValue(method, std::vector<unsigned>(1, 1));
							
							std::vector<llvm::Value*> parameters;
							
							SEM::Type* returnType = value->type();
							llvm::Value* returnValue = NULL;
							
							if (returnType->isClassOrTemplateVar()) {
								returnValue = genAlloca(returnType);
								assert(returnValue != NULL && "Must have lvalue for holding class return value so it can be passed by reference");
								parameters.push_back(returnValue);
							}
							
							parameters.push_back(contextPointer);
							
							const std::vector<SEM::Value*>& paramList = value->methodCall.parameters;
							
							for (std::size_t i = 0; i < paramList.size(); i++) {
								LOG(LOG_EXCESSIVE, "Generating method call argument %s.",
									paramList.at(i)->toString().c_str());
								parameters.push_back(genValue(paramList.at(i)));
							}
							
							LOG(LOG_EXCESSIVE, "Creating method call.");
							function->dump();
							contextPointer->dump();
							
							llvm::Value* callReturnValue = builder_.CreateCall(function, parameters);
							
							if (returnValue != NULL) {
								return genLoad(returnValue, returnType);
							} else {
								return callReturnValue;
							}
						}
						
						default:
							assert(false && "Unknown value enum");
							return llvm::UndefValue::get(llvm::Type::getVoidTy(llvm::getGlobalContext()));
					}
				}
				
		};
		
		CodeGenerator::CodeGenerator(const TargetInfo& targetInfo, const std::string& moduleName) {
			codeGen_ = new InternalCodeGen(targetInfo, moduleName);
		}
		
		CodeGenerator::~CodeGenerator() {
			delete codeGen_;
		}
		
		void CodeGenerator::applyOptimisations(size_t optLevel) {
			codeGen_->applyOptimisations(optLevel);
		}
		
		void CodeGenerator::genNamespace(SEM::Namespace* nameSpace) {
			codeGen_->genNamespaceTypePlaceholders(nameSpace);
			codeGen_->genNamespaceTypeMembers(nameSpace);
			codeGen_->genNamespaceFunctionDecls(nameSpace);
			codeGen_->genNamespaceFunctionDefs(nameSpace);
		}
		
		void CodeGenerator::writeToFile(const std::string& fileName) {
			codeGen_->writeToFile(fileName);
		}
		
		void CodeGenerator::dumpToFile(const std::string& fileName) {
			codeGen_->dumpToFile(fileName);
		}
		
		void CodeGenerator::dump() {
			codeGen_->dump();
		}
		
	}
	
}

