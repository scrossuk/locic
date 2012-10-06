#include "llvm/Bitcode/ReaderWriter.h"
#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Basic/TargetInfo.h>

#include <assert.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <Locic/Map.hpp>
#include <Locic/SEM.hpp>
#include <Locic/CodeGen/CodeGen.hpp>

class CodeGen {
	private:
		std::string name_;
		llvm::Module* module_;
		llvm::IRBuilder<> builder_;
		llvm::FunctionType* currentFunctionType_;
		llvm::Function* currentFunction_;
		llvm::BasicBlock* currentBasicBlock_;
		Locic::Map<SEM::TypeInstance*, llvm::Type*> typeInstances_;
		Locic::Map<std::string, llvm::Function*> functions_;
		Locic::Map<std::string, std::size_t> primitiveSizes_;
		std::vector<llvm::AllocaInst*> localVariables_, paramVariables_;
		clang::TargetInfo* targetInfo_;
		llvm::Value * returnVar_;
		
	public:
		CodeGen(const std::string& moduleName)
			: name_(moduleName),
			  module_(new llvm::Module(name_.c_str(), llvm::getGlobalContext())),
			  builder_(llvm::getGlobalContext()),
			  targetInfo_(0),
			  returnVar_(NULL){
			  
			llvm::InitializeNativeTarget();
			  
			//std::cout << "Default target triple: " << llvm::sys::getDefaultTargetTriple() << std::endl;
			
			module_->setTargetTriple(llvm::sys::getDefaultTargetTriple());
			
			std::string error;
			const llvm::Target* target = llvm::TargetRegistry::lookupTarget(llvm::sys::getDefaultTargetTriple(), error);
			
			if(target != NULL) {
				/*std::cout << "Target: name=" << target->getName() << ", description=" << target->getShortDescription() << std::endl;
				
				std::cout << "--Does " << (target->hasJIT() ? "" : "not ") << "support just-in-time compilation" << std::endl;
				std::cout << "--Does " << (target->hasTargetMachine() ? "" : "not ") << "support code generation" << std::endl;
				std::cout << "--Does " << (target->hasMCAsmBackend() ? "" : "not ") << "support .o generation" << std::endl;
				std::cout << "--Does " << (target->hasMCAsmLexer() ? "" : "not ") << "support .s lexing" << std::endl;
				std::cout << "--Does " << (target->hasMCAsmParser() ? "" : "not ") << "support .s parsing" << std::endl;
				std::cout << "--Does " << (target->hasAsmPrinter() ? "" : "not ") << "support .s printing" << std::endl;
				std::cout << "--Does " << (target->hasMCDisassembler() ? "" : "not ") << "support disassembling" << std::endl;
				std::cout << "--Does " << (target->hasMCInstPrinter() ? "" : "not ") << "support printing instructions" << std::endl;
				std::cout << "--Does " << (target->hasMCCodeEmitter() ? "" : "not ") << "support instruction encoding" << std::endl;
				std::cout << "--Does " << (target->hasMCObjectStreamer() ? "" : "not ") << "support streaming to files" << std::endl;
				std::cout << "--Does " << (target->hasAsmStreamer() ? "" : "not ") << "support streaming ASM to files" << std::endl;*/
				
				if(target->hasTargetMachine()) {
					std::auto_ptr<llvm::TargetMachine> targetMachine(target->createTargetMachine(llvm::sys::getDefaultTargetTriple(), "", "", llvm::TargetOptions()));
					const llvm::TargetData* targetData = targetMachine->getTargetData();
					
					if(targetData != 0) {
						/*std::cout << "--Pointer size = " << targetData->getPointerSize() << std::endl;
						std::cout << "--Pointer size (in bits) = " << targetData->getPointerSizeInBits() << std::endl;
						std::cout << "--Little endian = " << (targetData->isLittleEndian() ? "true" : "false") << std::endl;
						std::cout << "--Big endian = " << (targetData->isBigEndian() ? "true" : "false") << std::endl;
						std::cout << "--Legal integer sizes = {";
						
						bool b = false;
						
						for(unsigned int i = 0; i < 1000; i++) {
							if(targetData->isLegalInteger(i)) {
								if(b) {
									std::cout << ", ";
								}
								
								std::cout << i;
								b = true;
							}
						}
						
						std::cout << "}" << std::endl;
						std::cout << std::endl;*/
						
						clang::CompilerInstance ci;
						ci.createDiagnostics(0, NULL);
						
						clang::TargetOptions to;
						to.Triple = llvm::sys::getDefaultTargetTriple();
						targetInfo_ = clang::TargetInfo::CreateTargetInfo(ci.getDiagnostics(), to);
						
						/*std::cout << "Information from Clang:" << std::endl;
						std::cout << "--Short width: " << targetInfo_->getShortWidth() << ", " << (sizeof(short) * 8) << std::endl;
						std::cout << "--Int width: " << targetInfo_->getIntWidth() << ", " << (sizeof(int) * 8) << std::endl;
						std::cout << "--Long width: " << targetInfo_->getLongWidth() << ", " << (sizeof(long) * 8) << std::endl;
						std::cout << "--Long long width: " << targetInfo_->getLongLongWidth() << ", " << (sizeof(long long) * 8) << std::endl;
						std::cout << "--Float width: " << targetInfo_->getFloatWidth() << ", " << (sizeof(float) * 8) << std::endl;
						std::cout << "--Double width: " << targetInfo_->getDoubleWidth() << ", " << (sizeof(double) * 8) << std::endl;
						std::cout << std::endl;*/
						
						primitiveSizes_.insert("short", targetInfo_->getShortWidth());
						primitiveSizes_.insert("int", targetInfo_->getIntWidth());
						primitiveSizes_.insert("long", targetInfo_->getLongWidth());
						primitiveSizes_.insert("longlong", targetInfo_->getLongLongWidth());
					}
				}
			} else {
				std::cout << "Error when looking up default target: " << error << std::endl;
			}
		}
		
		~CodeGen() {
			delete module_;
		}
		
		void dump() {
			module_->dump();
		}
		
		void dumpToFile(const std::string& fileName){
			std::ofstream file(fileName.c_str());
			llvm::raw_os_ostream ostream(file);
			ostream << *(module_);
		}
		
		void writeToFile(const std::string& fileName){
			std::ofstream file(fileName.c_str());
			llvm::raw_os_ostream ostream(file);
			llvm::WriteBitcodeToFile(module_, ostream);
		}
		
		void genBuiltInTypes(){
			std::vector< std::pair<std::string, std::size_t> > sizes;
			sizes.push_back(std::make_pair("short", targetInfo_->getShortWidth()));
			sizes.push_back(std::make_pair("int", targetInfo_->getIntWidth()));
			sizes.push_back(std::make_pair("long", targetInfo_->getLongWidth()));
			sizes.push_back(std::make_pair("longlong", targetInfo_->getLongLongWidth()));
			
			llvm::Type * boolType = llvm::Type::getInt1Ty(llvm::getGlobalContext());
			
			// Generate integer methods.
			for(std::size_t i = 0; i < sizes.size(); i++){
				const std::string name = sizes.at(i).first;
				const std::size_t size = sizes.at(i).second;
				
				llvm::Type * intType = llvm::IntegerType::get(llvm::getGlobalContext(), size);
				llvm::Type * ptrType = intType->getPointerTo();
				
				{
					const std::string functionName = name + "__implicitCopy";
					llvm::FunctionType * functionType = llvm::FunctionType::get(intType, std::vector<llvm::Type *>(1, ptrType), false);
					llvm::Function * function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
					
					llvm::BasicBlock * basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
					builder_.SetInsertPoint(basicBlock);
					builder_.CreateRet(builder_.CreateLoad(function->arg_begin()));
					functions_.insert(functionName, function);
				}
				
				{
					const std::string functionName = name + "__operatorAdd";
					std::vector<llvm::Type *> argumentTypes;
					argumentTypes.push_back(ptrType);
					argumentTypes.push_back(intType);
					llvm::FunctionType * functionType = llvm::FunctionType::get(intType, argumentTypes, false);
					llvm::Function * function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
					
					llvm::BasicBlock * basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
					builder_.SetInsertPoint(basicBlock);
					llvm::Function::arg_iterator arg = function->arg_begin();
					llvm::Value * firstArg = builder_.CreateLoad(arg++);
					llvm::Value * secondArg = arg;
					
					builder_.CreateRet(builder_.CreateAdd(firstArg, secondArg));
					functions_.insert(functionName, function);
				}
				
				{
					const std::string functionName = name + "__operatorSubtract";
					std::vector<llvm::Type *> argumentTypes;
					argumentTypes.push_back(ptrType);
					argumentTypes.push_back(intType);
					llvm::FunctionType * functionType = llvm::FunctionType::get(intType, argumentTypes, false);
					llvm::Function * function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
					
					llvm::BasicBlock * basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
					builder_.SetInsertPoint(basicBlock);
					llvm::Function::arg_iterator arg = function->arg_begin();
					llvm::Value * firstArg = builder_.CreateLoad(arg++);
					llvm::Value * secondArg = arg;
					
					builder_.CreateRet(builder_.CreateSub(firstArg, secondArg));
					functions_.insert(functionName, function);
				}
				
				{
					const std::string functionName = name + "__operatorMultiply";
					std::vector<llvm::Type *> argumentTypes;
					argumentTypes.push_back(ptrType);
					argumentTypes.push_back(intType);
					llvm::FunctionType * functionType = llvm::FunctionType::get(intType, argumentTypes, false);
					llvm::Function * function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
					
					llvm::BasicBlock * basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
					builder_.SetInsertPoint(basicBlock);
					llvm::Function::arg_iterator arg = function->arg_begin();
					llvm::Value * firstArg = builder_.CreateLoad(arg++);
					llvm::Value * secondArg = arg;
					
					builder_.CreateRet(builder_.CreateMul(firstArg, secondArg));
					functions_.insert(functionName, function);
				}
				
				{
					const std::string functionName = name + "__operatorDivide";
					std::vector<llvm::Type *> argumentTypes;
					argumentTypes.push_back(ptrType);
					argumentTypes.push_back(intType);
					llvm::FunctionType * functionType = llvm::FunctionType::get(intType, argumentTypes, false);
					llvm::Function * function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
					
					llvm::BasicBlock * basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
					builder_.SetInsertPoint(basicBlock);
					llvm::Function::arg_iterator arg = function->arg_begin();
					llvm::Value * firstArg = builder_.CreateLoad(arg++);
					llvm::Value * secondArg = arg;
					
					builder_.CreateRet(builder_.CreateSDiv(firstArg, secondArg));
					functions_.insert(functionName, function);
				}
				
				{
					const std::string functionName = name + "__operatorModulo";
					std::vector<llvm::Type *> argumentTypes;
					argumentTypes.push_back(ptrType);
					argumentTypes.push_back(intType);
					llvm::FunctionType * functionType = llvm::FunctionType::get(intType, argumentTypes, false);
					llvm::Function * function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
					
					llvm::BasicBlock * basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
					builder_.SetInsertPoint(basicBlock);
					llvm::Function::arg_iterator arg = function->arg_begin();
					llvm::Value * firstArg = builder_.CreateLoad(arg++);
					llvm::Value * secondArg = arg;
					
					builder_.CreateRet(builder_.CreateSRem(firstArg, secondArg));
					functions_.insert(functionName, function);
				}
				
				{
					const std::string functionName = name + "__operatorIsLess";
					std::vector<llvm::Type *> argumentTypes;
					argumentTypes.push_back(ptrType);
					argumentTypes.push_back(intType);
					llvm::FunctionType * functionType = llvm::FunctionType::get(boolType, argumentTypes, false);
					llvm::Function * function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
					
					llvm::BasicBlock * basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
					builder_.SetInsertPoint(basicBlock);
					llvm::Function::arg_iterator arg = function->arg_begin();
					llvm::Value * firstArg = builder_.CreateLoad(arg++);
					llvm::Value * secondArg = arg;
					
					builder_.CreateRet(builder_.CreateICmpSLT(firstArg, secondArg));
					functions_.insert(functionName, function);
				}
				
				{
					const std::string functionName = name + "__operatorIsGreater";
					std::vector<llvm::Type *> argumentTypes;
					argumentTypes.push_back(ptrType);
					argumentTypes.push_back(intType);
					llvm::FunctionType * functionType = llvm::FunctionType::get(boolType, argumentTypes, false);
					llvm::Function * function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
					
					llvm::BasicBlock * basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
					builder_.SetInsertPoint(basicBlock);
					llvm::Function::arg_iterator arg = function->arg_begin();
					llvm::Value * firstArg = builder_.CreateLoad(arg++);
					llvm::Value * secondArg = arg;
					
					builder_.CreateRet(builder_.CreateICmpSGT(firstArg, secondArg));
					functions_.insert(functionName, function);
				}
				
				{
					const std::string functionName = name + "__abs";
					llvm::FunctionType * functionType = llvm::FunctionType::get(intType, std::vector<llvm::Type *>(1, ptrType), false);
					llvm::Function * function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
					
					llvm::BasicBlock * basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
					builder_.SetInsertPoint(basicBlock);
					llvm::Function::arg_iterator arg = function->arg_begin();
					llvm::Value * firstArg = builder_.CreateLoad(arg++);
					
					// Generates: (value < 0) ? -value : value
					llvm::Value * lessThanZero = builder_.CreateICmpSLT(firstArg, llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(size, 0)));
					builder_.CreateRet(builder_.CreateSelect(lessThanZero, builder_.CreateNeg(firstArg), firstArg));
					functions_.insert(functionName, function);
				}
			}
		}
		
		void genFile(SEM::Module* module) {
			assert(module != NULL && "Generating a module requires a non-NULL SEM module object");
			
			llvm::FunctionPassManager functionPassManager(module_);
			llvm::PassManager modulePassManager;
			
			llvm::PassManagerBuilder passManagerBuilder;
			passManagerBuilder.OptLevel = 2;
			passManagerBuilder.Inliner = llvm::createFunctionInliningPass();
			
			passManagerBuilder.populateFunctionPassManager(functionPassManager);
			passManagerBuilder.populateModulePassManager(modulePassManager);
			
			functionPassManager.doInitialization();
			
			genBuiltInTypes();
			
			for(std::size_t i = 0; i < module->functions.size(); i++){
				llvm::Function * function = genFunctionDef(module->functions.at(i));
				if(function != NULL){
					functionPassManager.run(*function);
				}
			}
			
			for(std::size_t i = 0; i < module->typeInstances.size(); i++){
				SEM::TypeInstance * typeInstance = module->typeInstances.at(i);
				
				// TODO: generate class records.
				(void) typeInstance;
			}
			
			modulePassManager.run(*module_);
		}
		
		// Lazy generation - function declarations are only
		// generated when they are first used by code.
		llvm::Function * genFunctionDecl(SEM::Function * function){
			assert(function != NULL && "Generating a function declaration requires a non-NULL SEM function object");
			
			const std::string functionName = function->name.genString();
			Locic::Optional<llvm::Function *> optionalFunction = functions_.tryGet(functionName);
			if(optionalFunction.hasValue()) return optionalFunction.getValue();
			
			llvm::Function * functionDecl = llvm::Function::Create(genFunctionType(function->type), llvm::Function::ExternalLinkage, functionName, module_);
			
			if(function->type->functionType.returnType->isClass()){
				functionDecl->addAttribute(1, llvm::Attribute::StructRet);
			}
			
			functions_.insert(functionName, functionDecl);
			return functionDecl;
		}
		
		// Lazy generation - struct types are only
		// generated when they are first used by code.
		llvm::Type* genStructType(SEM::TypeInstance * typeInstance){
			assert(typeInstance != NULL && "Generating struct type requires non-NULL SEM TypeInstance object");
			assert(typeInstance->typeEnum != SEM::TypeInstance::PRIMITIVE && "Generating struct type requires non-primitive type");
			
			Locic::Optional<llvm::Type *> optionalStruct = typeInstances_.tryGet(typeInstance);
			if(optionalStruct.hasValue()) return optionalStruct.getValue();
			
			llvm::StructType * structType = llvm::StructType::create(llvm::getGlobalContext(), typeInstance->name.genString());
			
			// Add the struct type before setting its body, since the struct can contain
			// variables that have a type that contains this struct (e.g. struct contains
			// a pointer to itself, such as in a linked list).
			typeInstances_.insert(typeInstance, structType);
			
			if(typeInstance->typeEnum == SEM::TypeInstance::CLASSDEF || typeInstance->typeEnum == SEM::TypeInstance::STRUCT){
				// Generating the type for a class definition or struct, so
				// the size and contents of the type instance is known.
				
				// Classes have a record pointer (holding things like virtual functions)
				// which is the first member; structs have no such pointer.
				const std::size_t paramOffset = (typeInstance->typeEnum == SEM::TypeInstance::CLASSDEF) ? 1 : 0;
			
				std::vector<llvm::Type*> memberVariables(paramOffset + typeInstance->variables.size(), NULL);
				
				if(typeInstance->typeEnum == SEM::TypeInstance::CLASSDEF){
					// Add class record pointer.
					memberVariables.front() = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(llvm::getGlobalContext()));
				}
				
				Locic::StringMap<SEM::Var *>::Range range = typeInstance->variables.range();
				for(; !range.empty(); range.popFront()){
					SEM::Var * var = range.front().value();
					assert(memberVariables.at(paramOffset + var->id) == NULL && "Member variables must not share ids");
					memberVariables.at(paramOffset + var->id) = genType(var->type);
				}
				
				structType->setBody(memberVariables);
			}else{
				// Generating the type for a class declaration, so the size is
				// currently unknown (and will be known at load-time/run-time).
				std::vector<llvm::Type *> memberVariables;
				
				// Pointer to class record.
				memberVariables.push_back(llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(llvm::getGlobalContext())));
				
				// Zero length array indicates this structure is variable length,
				// or in other words, currently unknown for this code.
				memberVariables.push_back(llvm::ArrayType::get(llvm::Type::getInt8Ty(llvm::getGlobalContext()), 0));
				structType->setBody(memberVariables);
			}
			
			return structType;
		}
		
		llvm::FunctionType* genFunctionType(SEM::Type* type) {
			assert(type != NULL && "Generating a function type requires a non-NULL SEM Type object");
			assert(type->typeEnum == SEM::Type::FUNCTION && "Type must be a function type for it to be generated as such");
			
			SEM::Type * semReturnType = type->functionType.returnType;
			assert(semReturnType != NULL && "Generating function return type requires a non-NULL SEM return type");
			
			llvm::Type* returnType = genType(semReturnType);
			
			std::vector<llvm::Type*> paramTypes;
			
			if(semReturnType->isClass()){
				paramTypes.push_back(returnType->getPointerTo());
				returnType = llvm::Type::getVoidTy(llvm::getGlobalContext());
			}
			
			const std::vector<SEM::Type *>& params = type->functionType.parameterTypes;
			
			for(std::size_t i = 0; i < params.size(); i++){
				paramTypes.push_back(genType(params.at(i)));
			}
			
			return llvm::FunctionType::get(returnType, paramTypes, type->functionType.isVarArg);
		}
		
		llvm::Type * genPrimitiveType(SEM::TypeInstance * type){
			const std::string name = type->name.toString();
			if(name == "::bool") return llvm::Type::getInt1Ty(llvm::getGlobalContext());
			if(name == "::char") return llvm::Type::getInt8Ty(llvm::getGlobalContext());
			if(name == "::short") return llvm::IntegerType::get(llvm::getGlobalContext(), targetInfo_->getShortWidth());
			if(name == "::int") return llvm::IntegerType::get(llvm::getGlobalContext(), targetInfo_->getIntWidth());
			if(name == "::long") return llvm::IntegerType::get(llvm::getGlobalContext(), targetInfo_->getLongWidth());
			if(name == "::longlong") return llvm::IntegerType::get(llvm::getGlobalContext(), targetInfo_->getLongLongWidth());
			if(name == "::float") return llvm::Type::getFloatTy(llvm::getGlobalContext());
			if(name == "::double") return llvm::Type::getDoubleTy(llvm::getGlobalContext());
			assert(false && "Unrecognised primitive type");
			return NULL;
		}
		
		llvm::Type* genType(SEM::Type* type) {
			switch(type->typeEnum) {
				case SEM::Type::VOID: {
					return llvm::Type::getVoidTy(llvm::getGlobalContext());
				}
				case SEM::Type::NULLT: {
					return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(llvm::getGlobalContext()));
				}
				case SEM::Type::NAMED: {
					Locic::Name name = type->namedType.typeInstance->name;
					
					if(type->namedType.typeInstance->typeEnum == SEM::TypeInstance::PRIMITIVE){
						return genPrimitiveType(type->namedType.typeInstance);
					}else{
						return genStructType(type->namedType.typeInstance);
					}
				}
				case SEM::Type::POINTER: {
					llvm::Type* pointerType = genType(type->pointerType.targetType);
					
					if(pointerType->isVoidTy()) {
						// LLVM doesn't support 'void *' => use 'int8_t *' instead.
						return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(llvm::getGlobalContext()));
					} else {
						return pointerType->getPointerTo();
					}
				}
				case SEM::Type::FUNCTION: {
					return genFunctionType(type)->getPointerTo();
				}
				case SEM::Type::METHOD: {
					SEM::Type * objectType = SEM::Type::Named(SEM::Type::MUTABLE, SEM::Type::LVALUE, type->methodType.objectType);
					SEM::Type * pointerToObjectType = SEM::Type::Pointer(SEM::Type::MUTABLE, SEM::Type::LVALUE, objectType);
					
					std::vector<llvm::Type*> types;
					types.push_back(genFunctionType(type->methodType.functionType)->getPointerTo());
					types.push_back(genType(pointerToObjectType));
					return llvm::StructType::get(llvm::getGlobalContext(), types);
				}
				default: {
					assert(false && "Unknown type enum for generating type");
					return llvm::Type::getVoidTy(llvm::getGlobalContext());
				}
			}
		}
		
		llvm::Function * genFunctionDef(SEM::Function* function) {
			assert(function != NULL && "Generating a function definition requires a non-NULL SEM Function object");
			if(function->scope == NULL) return NULL;
		
			currentFunction_ = genFunctionDecl(function);
			assert(currentFunction_ != NULL && "Generating a function definition requires a valid declaration");
			
			currentBasicBlock_ = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", currentFunction_);
			builder_.SetInsertPoint(currentBasicBlock_);
			
			// Store arguments onto stack.
			llvm::Function::arg_iterator arg = currentFunction_->arg_begin();
			
			SEM::Type * returnType = function->type->functionType.returnType;
			
			if(returnType->isClass()){
				returnVar_ = arg++;
			}else{
				returnVar_ = NULL;
			}
			
			const std::vector<SEM::Var *>& parameterVars = function->parameters;
			
			for(std::size_t i = 0; i < parameterVars.size(); ++arg, i++){
				SEM::Var* paramVar = parameterVars.at(i);
				
				// Create an alloca for this variable.
				llvm::AllocaInst* stackObject = builder_.CreateAlloca(genType(paramVar->type));
				
				assert(paramVar->id == paramVariables_.size()
					&& "Parameter variables' ids should match their position in the parameter variable array");
				paramVariables_.push_back(stackObject);
				
				// Store the initial value into the alloca.
				builder_.CreateStore(arg, stackObject);
			}
			
			genScope(function->scope);
			
			// Need to terminate the final basic block.
			// (just make it loop to itself - this will
			// be removed by dead code elimination)
			builder_.CreateBr(builder_.GetInsertBlock());
			
			// Check the generated function is correct.
			verifyFunction(*currentFunction_);
			
			paramVariables_.clear();
			localVariables_.clear();
			
			return currentFunction_;
		}
		
		void genScope(SEM::Scope* scope) {
			for(std::size_t i = 0; i < scope->localVariables.size(); i++) {
				SEM::Var* localVar = scope->localVariables.at(i);
				
				// Create an alloca for this variable.
				llvm::AllocaInst* stackObject = builder_.CreateAlloca(genType(localVar->type));
				
				assert(localVar->id == localVariables_.size()
					&& "Local variables' ids should match their position in the local variable array");
				localVariables_.push_back(stackObject);
			}
			
			for(std::size_t i = 0; i < scope->statementList.size(); i++){		
				genStatement(scope->statementList.at(i));
			}
		}
		
		void genStatement(SEM::Statement* statement) {
			switch(statement->typeEnum) {
				case SEM::Statement::VALUE: {
					genValue(statement->valueStmt.value);
					break;
				}
				case SEM::Statement::SCOPE: {
					genScope(statement->scopeStmt.scope);
					break;
				}
				case SEM::Statement::IF: {
					llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "then", currentFunction_);
					llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "else");
					llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "ifmerge");
					
					builder_.CreateCondBr(genValue(statement->ifStmt.condition), thenBB, elseBB);
					
					// Create 'then'.
					builder_.SetInsertPoint(thenBB);
					
					genScope(statement->ifStmt.ifTrue);
					
					builder_.CreateBr(mergeBB);
					
					// Create 'else'.
					currentFunction_->getBasicBlockList().push_back(elseBB);
					builder_.SetInsertPoint(elseBB);
					
					if(statement->ifStmt.ifFalse != NULL) {
						genScope(statement->ifStmt.ifFalse);
					}
					
					builder_.CreateBr(mergeBB);
					
					// Create merge.
					currentFunction_->getBasicBlockList().push_back(mergeBB);
					builder_.SetInsertPoint(mergeBB);
					break;
				}
				case SEM::Statement::WHILE: {
					llvm::BasicBlock* insideLoopBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "insideLoop", currentFunction_);
					llvm::BasicBlock* afterLoopBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "afterLoop");
					
					builder_.CreateCondBr(genValue(statement->whileStmt.condition), insideLoopBB, afterLoopBB);
					
					// Create loop contents.
					builder_.SetInsertPoint(insideLoopBB);
					
					genScope(statement->whileStmt.whileTrue);
					
					builder_.CreateCondBr(genValue(statement->whileStmt.condition), insideLoopBB, afterLoopBB);
					
					// Create 'else'.
					currentFunction_->getBasicBlockList().push_back(afterLoopBB);
					builder_.SetInsertPoint(afterLoopBB);
					break;
				}
				case SEM::Statement::ASSIGN: {
					SEM::Value* lValue = statement->assignStmt.lValue;
					SEM::Value* rValue = statement->assignStmt.rValue;
					
					builder_.CreateStore(genValue(rValue), genValue(lValue, true));
					break;
				}
				case SEM::Statement::RETURN: {
					if(statement->returnStmt.value != NULL && statement->returnStmt.value->type->typeEnum != SEM::Type::VOID) {
						llvm::Value * returnValue = genValue(statement->returnStmt.value);
						if(returnVar_ != NULL){
							builder_.CreateStore(returnValue, returnVar_);
							builder_.CreateRetVoid();
						}else{
							builder_.CreateRet(returnValue);
						}
					} else {
						builder_.CreateRetVoid();
					}
					
					// Need a basic block after a return statement in case anything more is generated.
					// This (and any following code) will be removed by dead code elimination.
					builder_.SetInsertPoint(llvm::BasicBlock::Create(llvm::getGlobalContext(), "next", currentFunction_));
					break;
				}
				default:
					assert(false && "Unknown statement type");
					break;
			}
		}
		
		llvm::Value * generateLValue(SEM::Value * value){
			if(value->type->isLValue){
				return genValue(value, true);
			}else{
				llvm::Value * lValue = builder_.CreateAlloca(genType(value->type));
				llvm::Value * rValue = genValue(value);
				builder_.CreateStore(rValue, lValue);
				return lValue;
			}
		}
		
		llvm::Value* genValue(SEM::Value* value, bool genLValue = false) {
			switch(value->typeEnum) {
				case SEM::Value::CONSTANT: {
					switch(value->constant.typeEnum) {
						case SEM::Value::Constant::BOOLEAN:
							return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(1, value->constant.boolConstant));
						case SEM::Value::Constant::INTEGER:
							return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(targetInfo_->getIntWidth(), value->constant.intConstant));
						case SEM::Value::Constant::FLOAT:
							return llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(value->constant.floatConstant));
						case SEM::Value::Constant::CSTRING:
						{
							const std::string& stringValue = value->constant.stringConstant;
							const bool isConstant = true;
							llvm::ArrayType * arrayType = llvm::ArrayType::get(llvm::Type::getInt8Ty(llvm::getGlobalContext()), stringValue.size() + 1);
							llvm::Constant * constArray = llvm::ConstantDataArray::getString(llvm::getGlobalContext(), stringValue.c_str());
							llvm::GlobalVariable* globalArray = new llvm::GlobalVariable(*module_, arrayType, isConstant, llvm::GlobalValue::PrivateLinkage, constArray, "");
							globalArray->setAlignment(1);
							
							// Convert array to a pointer.
							return builder_.CreateConstGEP2_32(globalArray, 0, 0);
						}
						case SEM::Value::Constant::NULLVAL:
							return llvm::ConstantPointerNull::get(llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(llvm::getGlobalContext())));
						default:
							assert(false && "Unknown constant type");
							return llvm::UndefValue::get(llvm::Type::getVoidTy(llvm::getGlobalContext()));
					}
				}
				case SEM::Value::COPY: {
					return genValue(value->copyValue.value);
				}
				case SEM::Value::VAR: {
					SEM::Var* var = value->varValue.var;
					
					switch(var->typeEnum) {
						case SEM::Var::PARAM: {
							llvm::Value * val = paramVariables_.at(var->id);
							assert(val != NULL && "Parameter variable must exist to be referenced");
							if(genLValue) {
								return val;
							} else {
								return builder_.CreateLoad(val);
							}
						}
						case SEM::Var::LOCAL: {
							llvm::Value * val = localVariables_.at(var->id);
							assert(val != NULL && "Local variable must exist to be referenced");
							if(genLValue) {
								return val;
							} else {
								return builder_.CreateLoad(val);
							}
						}
						case SEM::Var::MEMBER: {
							assert(!paramVariables_.empty() && "There must be at least one parameter variable (which should contain the 'this' pointer)");
							llvm::Value * object = paramVariables_.front();
							
							llvm::Value * memberPtr = builder_.CreateConstInBoundsGEP2_32(builder_.CreateLoad(object), 0, var->id + 1);
							
							if(genLValue){
								return memberPtr;
							}else{
								return builder_.CreateLoad(memberPtr);
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
				case SEM::Value::DEREF: {
					if(genLValue) {
						return genValue(value->deref.value);
					} else {
						return builder_.CreateLoad(genValue(value->deref.value));
					}
				}
				case SEM::Value::TERNARY: {
					return builder_.CreateSelect(genValue(value->ternary.condition), genValue(value->ternary.ifTrue, genLValue), genValue(value->ternary.ifFalse, genLValue));
				}
				case SEM::Value::CAST: {
					llvm::Value* codeValue = genValue(value->cast.value, genLValue);
					SEM::Type* sourceType = value->cast.value->type;
					SEM::Type* destType = value->type;
					
					assert((sourceType->typeEnum == destType->typeEnum || sourceType->typeEnum == SEM::Type::NULLT ||
						destType->typeEnum == SEM::Type::VOID) && "Types must be in the same group for cast, or it should be a cast from null, or a cast to void");
					
					if(destType->typeEnum == SEM::Type::VOID){
						// All casts to void have the same outcome.
						return llvm::UndefValue::get(llvm::Type::getVoidTy(llvm::getGlobalContext()));
					}
					
					switch(sourceType->typeEnum) {
						case SEM::Type::VOID: {
							return codeValue;
						}
						case SEM::Type::NULLT: {
							switch(destType->typeEnum){
								case SEM::Type::NULLT:
									return codeValue;
								case SEM::Type::POINTER:
								case SEM::Type::FUNCTION:
									return builder_.CreatePointerCast(codeValue, genType(destType));
								case SEM::Type::NAMED:
								{
									SEM::TypeInstance * typeInstance = destType->namedType.typeInstance;
									assert(typeInstance->isClass());
									
									llvm::Type * structType = genStructType(typeInstance);
									llvm::Value * structValue = llvm::UndefValue::get(structType);
									
									// Set class record pointer to NULL.
									llvm::Value * nullRecordPointer = llvm::ConstantPointerNull::get(llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(llvm::getGlobalContext())));
									
									structValue = builder_.CreateInsertValue(structValue, nullRecordPointer, std::vector<unsigned>(1, 0));
									
									return structValue;
								}
								default:
								{
									assert(false && "Invalid cast from null");
									return llvm::UndefValue::get(llvm::Type::getVoidTy(llvm::getGlobalContext()));
								}
							}
						}
						case SEM::Type::NAMED:
						{
							if(sourceType->namedType.typeInstance == destType->namedType.typeInstance){
								return codeValue;
							}
							
							assert(false && "Casts between named types not implemented");
							return NULL;
						}
						case SEM::Type::POINTER: {
							if(genLValue) {
								return builder_.CreatePointerCast(codeValue, llvm::PointerType::getUnqual(genType(destType)));
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
						default:
							assert(false && "Unknown type in cast");
							return llvm::UndefValue::get(llvm::Type::getVoidTy(llvm::getGlobalContext()));
					}
				}
				case SEM::Value::MEMBERACCESS:
				{
					if(genLValue){
						return builder_.CreateConstInBoundsGEP2_32(genValue(value->memberAccess.object, true), 0, value->memberAccess.memberId);
					}else{
						return builder_.CreateExtractValue(genValue(value->memberAccess.object), std::vector<unsigned>(1, value->memberAccess.memberId));
					}
				}
				case SEM::Value::FUNCTIONCALL: {
					std::vector<llvm::Value*> parameters;
					
					const std::vector<SEM::Value *>& paramList = value->functionCall.parameters;
					
					SEM::Type * returnType = value->type;
					llvm::Value * returnValue = NULL;
					
					if(returnType->isClass()){
						returnValue = builder_.CreateAlloca(genType(returnType));
						assert(returnValue != NULL && "Must have lvalue for holding class return value so it can be passed by reference");
						parameters.push_back(returnValue);
					}
					
					for(std::size_t i = 0; i < paramList.size(); i++){
						parameters.push_back(genValue(paramList.at(i)));
					}
					
					llvm::Value * function = genValue(value->functionCall.functionValue);
					llvm::Value * callReturnValue = builder_.CreateCall(function, parameters);
					
					if(returnValue != NULL){
						return builder_.CreateLoad(returnValue);
					}else{
						return callReturnValue;
					}
				}
				case SEM::Value::FUNCTIONREF: {
					llvm::Function* function = genFunctionDecl(value->functionRef.function);
					assert(function != NULL && "FunctionRef requires a valid function");
					return function;
				}
				case SEM::Value::METHODOBJECT: {
					llvm::Function* function = genFunctionDecl(value->methodObject.method);
					assert(function != NULL && "MethodObject requires a valid function");
					
					llvm::Value* dataPointer = generateLValue(value->methodObject.methodOwner);
					assert(dataPointer != NULL && "MethodObject requires a valid data pointer");
					
					llvm::Value * methodValue = llvm::UndefValue::get(genType(value->type));
					methodValue = builder_.CreateInsertValue(methodValue, function, std::vector<unsigned>(1, 0));
					methodValue = builder_.CreateInsertValue(methodValue, dataPointer, std::vector<unsigned>(1, 1));
					return methodValue;
				}
				case SEM::Value::METHODCALL: {
					llvm::Value * method = genValue(value->methodCall.methodValue);
					
					llvm::Value * function = builder_.CreateExtractValue(method, std::vector<unsigned>(1, 0));
					llvm::Value * dataPointer = builder_.CreateExtractValue(method, std::vector<unsigned>(1, 1));
					
					std::vector<llvm::Value*> parameters;
					parameters.push_back(dataPointer);
					
					const std::vector<SEM::Value *>& paramList = value->methodCall.parameters;
					for(std::size_t i = 0; i < paramList.size(); i++){
						parameters.push_back(genValue(paramList.at(i)));
					}
					
					return builder_.CreateCall(function, parameters);
				}
				default:
					assert(false && "Unknown value enum");
					return llvm::UndefValue::get(llvm::Type::getVoidTy(llvm::getGlobalContext()));
			}
		}
		
};

void* Locic_CodeGenAlloc(const std::string& moduleName) {
	return new CodeGen(moduleName);
}

void Locic_CodeGenFree(void* context) {
	delete reinterpret_cast<CodeGen*>(context);
}
	
void Locic_CodeGen(void* context, SEM::Module* module) {
	reinterpret_cast<CodeGen*>(context)->genFile(module);
}
	
void Locic_CodeGenDump(void* context) {
	reinterpret_cast<CodeGen*>(context)->dump();
}

void Locic_CodeGenWriteToFile(void * context, const std::string& fileName){
	reinterpret_cast<CodeGen*>(context)->writeToFile(fileName);
}

void Locic_CodeGenDumpToFile(void * context, const std::string& fileName){
	reinterpret_cast<CodeGen*>(context)->dumpToFile(fileName);
}

