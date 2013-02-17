#include "llvm/Bitcode/ReaderWriter.h"
#include <llvm/DerivedTypes.h>
#include <llvm/InlineAsm.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/IRBuilder.h>
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
#include <Locic/CodeGen/TargetInfo.hpp>
#include <Locic/CodeGen/VTable.hpp>

namespace Locic {

	namespace CodeGen {
		
		Map<MethodHash, SEM::Function *> CreateFunctionHashMap(SEM::TypeInstance * typeInstance){
			Map<MethodHash, SEM::Function *> hashMap;
			
			StringMap<SEM::Function*>::Range range = typeInstance->functions.range();
			
			for(; !range.empty(); range.popFront()) {
				SEM::Function* function = range.front().value();
				const std::string& name = function->name.last();
				hashMap.insert(CreateMethodNameHash(name), function);
			}
			
			assert(hashMap.size() == typeInstance->functions.size());
			
			return hashMap;
		}
		
		std::vector<MethodHash> CreateHashArray(const Map<MethodHash, SEM::Function *>& hashMap){
			std::vector<MethodHash> hashArray;
			
			Map<MethodHash, SEM::Function*>::Range range = hashMap.range();
			
			for(; !range.empty(); range.popFront()) {
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
				Locic::Map<SEM::TypeInstance*, llvm::Type*> typeInstances_;
				Locic::Map<std::string, llvm::Function*> functions_;
				std::vector<llvm::Value*> localVariables_, paramVariables_;
				llvm::Value* returnVar_;
				llvm::Value* thisPointer_;
				llvm::Function* memcpy_;
				llvm::StructType* vtableType_;
				
			public:
				InternalCodeGen(const TargetInfo& targetInfo, const std::string& moduleName)
					: name_(moduleName),
					  targetInfo_(targetInfo),
					  module_(new llvm::Module(name_.c_str(), llvm::getGlobalContext())),
					  builder_(llvm::getGlobalContext()),
					  returnVar_(NULL),
					  thisPointer_(NULL),
					  memcpy_(NULL),
					  vtableType_(NULL) {
					module_->setTargetTriple(targetInfo_.getTargetTriple());
					genBuiltInTypes();
				}
				
				~InternalCodeGen() {
					delete module_;
				}
				
				void dump() {
					module_->dump();
				}
				
				void dumpToFile(const std::string& fileName) {
					std::ofstream file(fileName.c_str());
					llvm::raw_os_ostream ostream(file);
					ostream << *(module_);
				}
				
				void writeToFile(const std::string& fileName) {
					std::ofstream file(fileName.c_str());
					llvm::raw_os_ostream ostream(file);
					llvm::WriteBitcodeToFile(module_, ostream);
				}
				
				void genBuiltInTypes() {
					std::vector< std::pair<std::string, std::size_t> > sizes;
					sizes.push_back(std::make_pair("short", targetInfo_.getPrimitiveSize("short")));
					sizes.push_back(std::make_pair("int", targetInfo_.getPrimitiveSize("int")));
					sizes.push_back(std::make_pair("long", targetInfo_.getPrimitiveSize("long")));
					sizes.push_back(std::make_pair("longlong", targetInfo_.getPrimitiveSize("longlong")));
					const size_t integerWidth = targetInfo_.getPrimitiveSize("int");
					llvm::Type* boolType = llvm::Type::getInt1Ty(llvm::getGlobalContext());
					llvm::Type* integerType = llvm::IntegerType::get(llvm::getGlobalContext(), integerWidth);
					{
						const std::string functionName = "bool__not";
						llvm::Type* ptrType = boolType->getPointerTo();
						llvm::FunctionType* functionType = llvm::FunctionType::get(boolType, std::vector<llvm::Type*>(1, ptrType), false);
						llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
						llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
						builder_.SetInsertPoint(basicBlock);
						builder_.CreateRet(builder_.CreateNot(builder_.CreateLoad(function->arg_begin())));
						functions_.insert(functionName, function);
					}
					
					// Generate integer methods.
					for(std::size_t i = 0; i < sizes.size(); i++) {
						const std::string name = sizes.at(i).first;
						const std::size_t size = sizes.at(i).second;
						llvm::Type* intType = llvm::IntegerType::get(llvm::getGlobalContext(), size);
						llvm::Type* ptrType = intType->getPointerTo();
						{
							const std::string functionName = name + "__implicitCopy";
							llvm::FunctionType* functionType = llvm::FunctionType::get(intType, std::vector<llvm::Type*>(1, ptrType), false);
							llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
							llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
							builder_.SetInsertPoint(basicBlock);
							builder_.CreateRet(builder_.CreateLoad(function->arg_begin()));
							functions_.insert(functionName, function);
						}
						{
							const std::string functionName = name + "__add";
							std::vector<llvm::Type*> argumentTypes;
							argumentTypes.push_back(ptrType);
							argumentTypes.push_back(intType);
							llvm::FunctionType* functionType = llvm::FunctionType::get(intType, argumentTypes, false);
							llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
							llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
							builder_.SetInsertPoint(basicBlock);
							llvm::Function::arg_iterator arg = function->arg_begin();
							llvm::Value* firstArg = builder_.CreateLoad(arg++);
							llvm::Value* secondArg = arg;
							builder_.CreateRet(builder_.CreateAdd(firstArg, secondArg));
							functions_.insert(functionName, function);
						}
						{
							const std::string functionName = name + "__subtract";
							std::vector<llvm::Type*> argumentTypes;
							argumentTypes.push_back(ptrType);
							argumentTypes.push_back(intType);
							llvm::FunctionType* functionType = llvm::FunctionType::get(intType, argumentTypes, false);
							llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
							llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
							builder_.SetInsertPoint(basicBlock);
							llvm::Function::arg_iterator arg = function->arg_begin();
							llvm::Value* firstArg = builder_.CreateLoad(arg++);
							llvm::Value* secondArg = arg;
							builder_.CreateRet(builder_.CreateSub(firstArg, secondArg));
							functions_.insert(functionName, function);
						}
						{
							const std::string functionName = name + "__multiply";
							std::vector<llvm::Type*> argumentTypes;
							argumentTypes.push_back(ptrType);
							argumentTypes.push_back(intType);
							llvm::FunctionType* functionType = llvm::FunctionType::get(intType, argumentTypes, false);
							llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
							llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
							builder_.SetInsertPoint(basicBlock);
							llvm::Function::arg_iterator arg = function->arg_begin();
							llvm::Value* firstArg = builder_.CreateLoad(arg++);
							llvm::Value* secondArg = arg;
							builder_.CreateRet(builder_.CreateMul(firstArg, secondArg));
							functions_.insert(functionName, function);
						}
						{
							const std::string functionName = name + "__divide";
							std::vector<llvm::Type*> argumentTypes;
							argumentTypes.push_back(ptrType);
							argumentTypes.push_back(intType);
							llvm::FunctionType* functionType = llvm::FunctionType::get(intType, argumentTypes, false);
							llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
							llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
							builder_.SetInsertPoint(basicBlock);
							llvm::Function::arg_iterator arg = function->arg_begin();
							llvm::Value* firstArg = builder_.CreateLoad(arg++);
							llvm::Value* secondArg = arg;
							builder_.CreateRet(builder_.CreateSDiv(firstArg, secondArg));
							functions_.insert(functionName, function);
						}
						{
							const std::string functionName = name + "__modulo";
							std::vector<llvm::Type*> argumentTypes;
							argumentTypes.push_back(ptrType);
							argumentTypes.push_back(intType);
							llvm::FunctionType* functionType = llvm::FunctionType::get(intType, argumentTypes, false);
							llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
							llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
							builder_.SetInsertPoint(basicBlock);
							llvm::Function::arg_iterator arg = function->arg_begin();
							llvm::Value* firstArg = builder_.CreateLoad(arg++);
							llvm::Value* secondArg = arg;
							builder_.CreateRet(builder_.CreateSRem(firstArg, secondArg));
							functions_.insert(functionName, function);
						}
						{
							const std::string functionName = name + "__compare";
							std::vector<llvm::Type*> argumentTypes;
							argumentTypes.push_back(ptrType);
							argumentTypes.push_back(intType);
							llvm::FunctionType* functionType = llvm::FunctionType::get(integerType, argumentTypes, false);
							llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
							llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
							builder_.SetInsertPoint(basicBlock);
							llvm::Function::arg_iterator arg = function->arg_begin();
							llvm::Value* firstArg = builder_.CreateLoad(arg++);
							llvm::Value* secondArg = arg;
							llvm::Value* isLessThan = builder_.CreateICmpSLT(firstArg, secondArg);
							llvm::Value* isGreaterThan = builder_.CreateICmpSGT(firstArg, secondArg);
							llvm::Value* minusOne = llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(integerWidth, -1));
							llvm::Value* zero = llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(integerWidth, 0));
							llvm::Value* plusOne = llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(integerWidth, 1));
							llvm::Value* returnValue =
								builder_.CreateSelect(isLessThan, minusOne,
													  builder_.CreateSelect(isGreaterThan, plusOne, zero));
							builder_.CreateRet(returnValue);
							functions_.insert(functionName, function);
						}
						{
							const std::string functionName = name + "__isZero";
							std::vector<llvm::Type*> argumentTypes;
							argumentTypes.push_back(ptrType);
							llvm::FunctionType* functionType = llvm::FunctionType::get(boolType, argumentTypes, false);
							llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
							llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
							builder_.SetInsertPoint(basicBlock);
							llvm::Value* arg = builder_.CreateLoad(function->arg_begin());
							llvm::Value* zero = llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(size, 0));
							builder_.CreateRet(builder_.CreateICmpEQ(arg, zero));
							functions_.insert(functionName, function);
						}
						{
							const std::string functionName = name + "__isPositive";
							std::vector<llvm::Type*> argumentTypes;
							argumentTypes.push_back(ptrType);
							llvm::FunctionType* functionType = llvm::FunctionType::get(boolType, argumentTypes, false);
							llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
							llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
							builder_.SetInsertPoint(basicBlock);
							llvm::Value* arg = builder_.CreateLoad(function->arg_begin());
							llvm::Value* zero = llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(size, 0));
							builder_.CreateRet(builder_.CreateICmpSGT(arg, zero));
							functions_.insert(functionName, function);
						}
						{
							const std::string functionName = name + "__isNegative";
							std::vector<llvm::Type*> argumentTypes;
							argumentTypes.push_back(ptrType);
							llvm::FunctionType* functionType = llvm::FunctionType::get(boolType, argumentTypes, false);
							llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
							llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
							builder_.SetInsertPoint(basicBlock);
							llvm::Value* arg = builder_.CreateLoad(function->arg_begin());
							llvm::Value* zero = llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(size, 0));
							builder_.CreateRet(builder_.CreateICmpSLT(arg, zero));
							functions_.insert(functionName, function);
						}
						{
							const std::string functionName = name + "__abs";
							llvm::FunctionType* functionType = llvm::FunctionType::get(intType, std::vector<llvm::Type*>(1, ptrType), false);
							llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
							llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
							builder_.SetInsertPoint(basicBlock);
							llvm::Function::arg_iterator arg = function->arg_begin();
							llvm::Value* firstArg = builder_.CreateLoad(arg++);
							// Generates: (value < 0) ? -value : value
							llvm::Value* lessThanZero = builder_.CreateICmpSLT(firstArg, llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(size, 0)));
							builder_.CreateRet(builder_.CreateSelect(lessThanZero, builder_.CreateNeg(firstArg), firstArg));
							functions_.insert(functionName, function);
						}
						{
							const size_t sizeTypeWidth = targetInfo_.getPrimitiveSize("size_t");
							llvm::Type* sizeType = llvm::IntegerType::get(llvm::getGlobalContext(), sizeTypeWidth);
							const std::string functionName = std::string("__BUILTIN__") + name + "__sizeof";
							llvm::FunctionType* functionType = llvm::FunctionType::get(sizeType, std::vector<llvm::Type*>(), false);
							llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, module_);
							llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
							builder_.SetInsertPoint(basicBlock);
							builder_.CreateRet(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(sizeTypeWidth, size / 8)));
							functions_.insert(functionName, function);
						}
					}
				}
				
				void applyOptimisations(size_t optLevel) {
					llvm::FunctionPassManager functionPassManager(module_);
					llvm::PassManager modulePassManager;
					llvm::PassManagerBuilder passManagerBuilder;
					passManagerBuilder.OptLevel = optLevel;
					passManagerBuilder.Inliner = llvm::createFunctionInliningPass();
					passManagerBuilder.populateFunctionPassManager(functionPassManager);
					passManagerBuilder.populateModulePassManager(modulePassManager);
					functionPassManager.doInitialization();
					llvm::Module::iterator i;
					
					for(i = module_->begin(); i != module_->end(); ++i) {
						functionPassManager.run(*i);
					}
					
					modulePassManager.run(*module_);
				}
				
				void genNamespace(SEM::Namespace* nameSpace) {
					Locic::StringMap<SEM::NamespaceNode>::Range range = nameSpace->children.range();
					
					for(; !range.empty(); range.popFront()) {
						SEM::NamespaceNode node = range.front().value();
						
						switch(node.typeEnum) {
							case SEM::NamespaceNode::FUNCTION: {
								genFunctionDef(node.getFunction());
								break;
							}
							case SEM::NamespaceNode::NAMESPACE: {
								genNamespace(node.getNamespace());
								break;
							}
							case SEM::NamespaceNode::TYPEINSTANCE: {
								genTypeInstance(node.getTypeInstance());
								break;
							}
							default:
								break;
						}
					}
				}
				
				llvm::Type* voidType() {
					return llvm::Type::getVoidTy(llvm::getGlobalContext());
				}
				
				llvm::Type* i8Type() {
					return llvm::Type::getInt8Ty(llvm::getGlobalContext());
				}
				
				llvm::Type* i32Type() {
					return llvm::Type::getInt32Ty(llvm::getGlobalContext());
				}
				
				llvm::PointerType* i8PtrType() {
					return i8Type()->getPointerTo();
				}
				
				llvm::StructType* getVTableType() {
					if(vtableType_ != NULL) return vtableType_;
					
					std::vector<llvm::Type*> structElements;
					// Destructor.
					const bool isVarArg = false;
					structElements.push_back(llvm::FunctionType::get(voidType(), std::vector<llvm::Type*>(1, i8PtrType()), isVarArg)
											 ->getPointerTo());
					structElements.push_back(llvm::ArrayType::get(i8PtrType(), VTABLE_SIZE));
					vtableType_ = llvm::StructType::create(llvm::getGlobalContext(), structElements, "__vtable_type");
					return vtableType_;
				}
				
				llvm::Value* genVTable(SEM::TypeInstance* typeInstance) {
					assert(typeInstance->isClass() && "Can only generate method vtable for class types");
					const std::string vtableName = makeString("__VTABLE__%s", typeInstance->name.genString().c_str());
					
					llvm::GlobalVariable * existingGlobalVariable = module_->getGlobalVariable(vtableName);
					if(existingGlobalVariable != NULL) return existingGlobalVariable;
					
					const bool isConstant = true;
					llvm::GlobalVariable * globalVariable = new llvm::GlobalVariable(*module_, getVTableType(),
						isConstant, llvm::GlobalValue::ExternalLinkage, NULL, vtableName);
					
					if(typeInstance->isClassDecl()) return globalVariable;
					
					// Generate the vtable.
					const Map<MethodHash, SEM::Function *> functionHashMap = CreateFunctionHashMap(typeInstance);
					std::vector<MethodHash> hashArray = CreateHashArray(functionHashMap);
					
					const VirtualTable virtualTable = VirtualTable::CalculateFromHashes(hashArray);
					
					std::vector<llvm::Constant *> vtableStructElements;
					
					// Destructor.
					const bool isVarArg = false;
					llvm::PointerType * destructorType = llvm::FunctionType::get(voidType(), std::vector<llvm::Type*>(1, i8PtrType()), isVarArg)
						->getPointerTo();
					vtableStructElements.push_back(llvm::ConstantPointerNull::get(destructorType));
					
					// Method slots.
					std::vector<llvm::Constant *> methodSlotElements;
					for(size_t i = 0; i < VTABLE_SIZE; i++){
						const std::list<MethodHash>& slotList = virtualTable.table().at(i);
						if(slotList.empty()){
							methodSlotElements.push_back(llvm::ConstantPointerNull::get(i8PtrType()));
						}else if(slotList.size() > 1){
							printf("COLLISION at %llu.\n",
								(unsigned long long) i);
							methodSlotElements.push_back(llvm::ConstantPointerNull::get(i8PtrType()));
						}else{
							assert(slotList.size() == 1);
							SEM::Function * semFunction = functionHashMap.get(slotList.front());
							llvm::Function * function = genFunctionDecl(semFunction);
							methodSlotElements.push_back(llvm::ConstantExpr::getPointerCast(function, i8PtrType()));
						}
					}
					
					llvm::Constant * methodSlotTable = llvm::ConstantArray::get(
						llvm::ArrayType::get(i8PtrType(), VTABLE_SIZE), methodSlotElements);
					vtableStructElements.push_back(methodSlotTable);
					
					llvm::Constant * vtableStruct =
						llvm::ConstantStruct::get(getVTableType(), vtableStructElements);
					
					globalVariable->setInitializer(vtableStruct);
					
					return globalVariable;
				}
				
				llvm::Function* genInterfaceMethod(SEM::Function* function) {
					assert(function->scope == NULL && "Interface methods must be empty");
					assert(function->isMethod && "Must be a method");
					llvm::Function* generatedFunction = genFunctionDecl(function);
					assert(generatedFunction != NULL && "Generating a function definition requires a valid declaration");
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", generatedFunction);
					builder_.SetInsertPoint(basicBlock);
					// Store arguments onto stack.
					llvm::Function::arg_iterator arg = generatedFunction->arg_begin();
					SEM::Type* returnType = function->type->functionType.returnType;
					llvm::Value* returnVar = returnType->isClass() ? arg++ : NULL;
					
					// Get the 'this' record, which is the
					// pair of the 'this' pointer and the
					// method vtable pointer.
					llvm::Value* thisRecord = arg++;
					
					// Get the 'this' pointer.
					llvm::Value* thisPointer = builder_.CreateExtractValue(thisRecord, std::vector<unsigned>(1, 0), "thisPointer");
					
					// Get the vtable pointer.
					llvm::Value* vtablePointer = builder_.CreateExtractValue(thisRecord, std::vector<unsigned>(1, 1), "vtablePointer");
					
					const Locic::CodeGen::MethodHash methodHash = Locic::CodeGen::CreateMethodNameHash(function->name.last());
					const size_t offset = methodHash % VTABLE_SIZE;
					
					std::vector<llvm::Value*> vtableEntryGEP;
					vtableEntryGEP.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, 0)));
					vtableEntryGEP.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, 1)));
					vtableEntryGEP.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, offset)));
					
					llvm::Value* vtableEntryPointer = builder_.CreateInBoundsGEP(vtablePointer, vtableEntryGEP, "vtableEntryPointer");
					llvm::Value* methodFunctionPointer = builder_.CreateLoad(vtableEntryPointer, "methodFunctionPointer");
					llvm::Type* methodFunctionType = genFunctionType(function->type, thisPointer->getType());
					llvm::Value* castedMethodFunctionPointer = builder_.CreatePointerCast(
								methodFunctionPointer, methodFunctionType->getPointerTo(), "castedMethodFunctionPointer");
					std::vector<llvm::Value*> arguments;
					
					if(returnVar != NULL) arguments.push_back(returnVar);
					
					arguments.push_back(thisPointer);
					
					while(arg != generatedFunction->arg_end()) arguments.push_back(arg++);
					
					llvm::FunctionType * asmFunctionType = llvm::FunctionType::get(voidType(), std::vector<llvm::Type*>(), false);
					std::string assembly = makeString("movl $$%llu, %%eax",
						(unsigned long long) methodHash);
					
					llvm::InlineAsm * setEax = llvm::InlineAsm::get(asmFunctionType, assembly, "~eax", true);
					builder_.CreateCall(setEax);
					
					llvm::Value* methodCallValue = builder_.CreateCall(castedMethodFunctionPointer,
												   arguments, returnType->isVoid() ? "" : "methodCallValue");
					
					if(returnType->isVoid()){
						builder_.CreateRetVoid();
					}else{
						builder_.CreateRet(methodCallValue);
					}
					
					// Check the generated function is correct.
					verifyFunction(*generatedFunction);
					return generatedFunction;
				}
				
				void genTypeInstance(SEM::TypeInstance* typeInstance) {
					if(typeInstance->typeEnum == SEM::TypeInstance::CLASSDEF) {
						// Make sure the 'sizeof' method is generated
						// for class definitions.
						(void) genSizeOfMethod(typeInstance);
						// Must generate the method vtable for
						// class definitions.
						(void) genVTable(typeInstance);
						Locic::StringMap<SEM::Function*>::Range range = typeInstance->functions.range();
						
						for(; !range.empty(); range.popFront()) {
							SEM::Function* function = range.front().value();
							(void) genFunctionDef(function);
						}
					} else if(typeInstance->typeEnum == SEM::TypeInstance::INTERFACE) {
						Locic::StringMap<SEM::Function*>::Range range = typeInstance->functions.range();
						
						for(; !range.empty(); range.popFront()) {
							SEM::Function* function = range.front().value();
							(void) genInterfaceMethod(function);
						}
					}
				}
				
				// Generate 'sizeof()' method.
				llvm::Function* genSizeOfMethod(SEM::TypeInstance* typeInstance) {
					const std::string functionName = std::string("__BUILTIN__") + typeInstance->name.genString() + "__sizeof";
					Locic::Optional<llvm::Function*> result = functions_.tryGet(functionName);
					
					if(result.hasValue()) return result.getValue();
					
					assert(!typeInstance->isPrimitive() && "Primitive types must have already defined a sizeof() function");
					const size_t sizeTypeWidth = targetInfo_.getPrimitiveSize("size_t");
					llvm::Type* sizeType = llvm::IntegerType::get(llvm::getGlobalContext(), sizeTypeWidth);
					llvm::FunctionType* functionType = llvm::FunctionType::get(sizeType, std::vector<llvm::Type*>(), false);
					// Struct definitions use 'LinkOnce' because they may
					// appear in multiple modules, and hence the compiler
					// may generate multiple sizeof() methods.
					llvm::GlobalValue::LinkageTypes linkage =
						(typeInstance->isClass() || typeInstance->isStructDecl())
						? llvm::Function::ExternalLinkage
						: llvm::Function::LinkOnceODRLinkage;
					llvm::Function* function = llvm::Function::Create(functionType, linkage, functionName, module_);
					function->setDoesNotAccessMemory();
					functions_.insert(functionName, function);
					
					if(typeInstance->isDeclaration()) {
						return function;
					}
					
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
					builder_.SetInsertPoint(basicBlock);
					llvm::Value* zero = llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(sizeTypeWidth, 0));
					llvm::Value* one = llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(sizeTypeWidth, 1));
					llvm::Value* classSize = zero;
					// Add up all member variable sizes.
					Locic::StringMap<SEM::Var*>::Range range = typeInstance->variables.range();
					
					for(; !range.empty(); range.popFront()) {
						SEM::Var* var = range.front().value();
						classSize = builder_.CreateAdd(classSize, genSizeOf(var->type));
					}
					
					// Class sizes must be at least one byte.
					llvm::Value* isZero = builder_.CreateICmpEQ(classSize, zero);
					classSize = builder_.CreateSelect(isZero, one, classSize);
					builder_.CreateRet(classSize);
					return function;
				}
				
				// Lazy generation - function declarations are only
				// generated when they are first used by code.
				llvm::Function* genFunctionDecl(SEM::Function* function) {
					assert(function != NULL && "Generating a function declaration requires a non-NULL SEM function object");
					const std::string functionName = function->name.genString();
					Locic::Optional<llvm::Function*> optionalFunction = functions_.tryGet(functionName);
					
					if(optionalFunction.hasValue()) return optionalFunction.getValue();
					
					SEM::TypeInstance* thisTypeInstance = function->isMethod ? function->parentType : NULL;
					
					llvm::Function* functionDecl = llvm::Function::Create(genFunctionType(function->type,
												   getTypeInstancePointer(thisTypeInstance)), llvm::Function::ExternalLinkage, functionName, module_);
												   
					if(function->type->functionType.returnType->isClass()) {
						// Class return values are allocated by the caller,
						// which passes a pointer to the callee. The caller
						// and callee must, for the sake of optimisation,
						// ensure that the following attributes hold...
						// Caller must ensure pointer is always valid.
						functionDecl->addAttribute(1, llvm::Attribute::StructRet);
						// Caller must ensure pointer does not alias with
						// any other arguments.
						functionDecl->addAttribute(1, llvm::Attribute::NoAlias);
						// Callee must not capture the pointer.
						functionDecl->addAttribute(1, llvm::Attribute::NoCapture);
					}
					
					functions_.insert(functionName, functionDecl);
					return functionDecl;
				}
				
				// Lazy generation - struct types are only
				// generated when they are first used by code.
				llvm::Type* genStructType(SEM::TypeInstance* typeInstance) {
					assert(typeInstance != NULL && "Generating struct type requires non-NULL SEM TypeInstance object");
					assert(typeInstance->typeEnum != SEM::TypeInstance::PRIMITIVE && "Generating struct type requires non-primitive type");
					Locic::Optional<llvm::Type*> optionalStruct = typeInstances_.tryGet(typeInstance);
					
					if(optionalStruct.hasValue()) return optionalStruct.getValue();
					
					llvm::StructType* structType = llvm::StructType::create(llvm::getGlobalContext(), typeInstance->name.genString());
					// Add the struct type before setting its body, since the struct can contain
					// variables that have a type that contains this struct (e.g. struct contains
					// a pointer to itself, such as in a linked list).
					typeInstances_.insert(typeInstance, structType);
					
					if(typeInstance->isClassDef() || typeInstance->isStructDef()) {
						// Generating the type for a class or struct definition, so
						// the size and contents of the type instance is known and
						// hence the contents can be specified.
						std::vector<llvm::Type*> memberVariables(typeInstance->variables.size(), NULL);
						Locic::StringMap<SEM::Var*>::Range range = typeInstance->variables.range();
						
						for(; !range.empty(); range.popFront()) {
							SEM::Var* var = range.front().value();
							assert(memberVariables.at(var->id) == NULL && "Member variables must not share ids");
							memberVariables.at(var->id) = genType(var->type);
						}
						
						structType->setBody(memberVariables);
					}
					
					return structType;
				}
				
				llvm::Type* getTypeInstancePointer(SEM::TypeInstance* typeInstance) {
					if(typeInstance == NULL) return NULL;
					SEM::Type* pointerType =
						SEM::Type::Pointer(SEM::Type::MUTABLE, SEM::Type::RVALUE,
										   SEM::Type::Named(SEM::Type::MUTABLE, SEM::Type::LVALUE, typeInstance));
					return genType(pointerType);
				}
				
				llvm::FunctionType* genFunctionType(SEM::Type* type, llvm::Type* thisPointerType = NULL) {
					assert(type != NULL && "Generating a function type requires a non-NULL SEM Type object");
					assert(type->typeEnum == SEM::Type::FUNCTION && "Type must be a function type for it to be generated as such");
					SEM::Type* semReturnType = type->functionType.returnType;
					assert(semReturnType != NULL && "Generating function return type requires a non-NULL SEM return type");
					llvm::Type* returnType = genType(semReturnType);
					std::vector<llvm::Type*> paramTypes;
					
					if(semReturnType->isClass()) {
						// Class return values are constructed on the caller's
						// stack, and given to the callee as a pointer.
						paramTypes.push_back(returnType->getPointerTo());
						returnType = llvm::Type::getVoidTy(llvm::getGlobalContext());
					}
					
					if(thisPointerType != NULL) {
						// Generating a method, so add the 'this' pointer.
						paramTypes.push_back(thisPointerType);
					}
					
					const std::vector<SEM::Type*>& params = type->functionType.parameterTypes;
					
					for(std::size_t i = 0; i < params.size(); i++) {
						SEM::Type* paramType = params.at(i);
						llvm::Type* rawType = genType(paramType);
						
						if(paramType->isObjectType()) {
							SEM::TypeInstance* typeInstance = paramType->getObjectType();
							
							if(typeInstance->isClass()) {
								rawType = rawType->getPointerTo();
							}
						}
						
						paramTypes.push_back(rawType);
					}
					
					return llvm::FunctionType::get(returnType, paramTypes, type->functionType.isVarArg);
				}
				
				llvm::Type* genPrimitiveType(SEM::TypeInstance* type) {
					const std::string name = type->name.toString();
					
					if(name == "::bool") return llvm::Type::getInt1Ty(llvm::getGlobalContext());
					
					if(name == "::char") return llvm::Type::getInt8Ty(llvm::getGlobalContext());
					
					if(name == "::short") return llvm::IntegerType::get(llvm::getGlobalContext(), targetInfo_.getPrimitiveSize("short"));
					
					if(name == "::int") return llvm::IntegerType::get(llvm::getGlobalContext(), targetInfo_.getPrimitiveSize("int"));
					
					if(name == "::long") return llvm::IntegerType::get(llvm::getGlobalContext(), targetInfo_.getPrimitiveSize("long"));
					
					if(name == "::longlong") return llvm::IntegerType::get(llvm::getGlobalContext(), targetInfo_.getPrimitiveSize("longlong"));
					
					if(name == "::float") return llvm::Type::getFloatTy(llvm::getGlobalContext());
					
					if(name == "::double") return llvm::Type::getDoubleTy(llvm::getGlobalContext());
					
					if(name == "::longdouble") return llvm::Type::getFP128Ty(llvm::getGlobalContext());
					
					assert(false && "Unrecognised primitive type");
					return NULL;
				}
				
				llvm::Type* genType(SEM::Type* type) {
					switch(type->typeEnum) {
						case SEM::Type::VOID: {
							return llvm::Type::getVoidTy(llvm::getGlobalContext());
						}
						case SEM::Type::NULLT: {
							return llvm::Type::getInt8Ty(llvm::getGlobalContext())->getPointerTo();
						}
						case SEM::Type::NAMED: {
							Locic::Name name = type->namedType.typeInstance->name;
							SEM::TypeInstance* typeInstance = type->namedType.typeInstance;
							
							if(typeInstance->isPrimitive()) {
								return genPrimitiveType(type->namedType.typeInstance);
							} else {
								assert(!typeInstance->isInterface() && "Interface types must always be converted by pointer");
								return genStructType(type->namedType.typeInstance);
							}
						}
						case SEM::Type::POINTER: {
							SEM::Type* targetType = type->pointerType.targetType;
							
							// Interface pointers are actually two pointers: one
							// to the class, and one to the class vtable.
							if(targetType->isInterface()) {
								std::vector<llvm::Type*> types;
								// Class pointer.
								types.push_back(genStructType(targetType->getObjectType())->getPointerTo());
								// Vtable pointer.
								types.push_back(getVTableType()->getPointerTo());
								return llvm::StructType::get(llvm::getGlobalContext(), types);
							}
							
							llvm::Type* pointerType = genType(targetType);
							
							if(pointerType->isVoidTy()) {
								// LLVM doesn't support 'void *' => use 'int8_t *' instead.
								return llvm::Type::getInt8Ty(llvm::getGlobalContext())->getPointerTo();
							} else {
								return pointerType->getPointerTo();
							}
						}
						case SEM::Type::FUNCTION: {
							return genFunctionType(type)->getPointerTo();
						}
						case SEM::Type::METHOD: {
							SEM::Type* objectType = SEM::Type::Named(SEM::Type::MUTABLE, SEM::Type::LVALUE, type->methodType.objectType);
							SEM::Type* pointerToObjectType = SEM::Type::Pointer(SEM::Type::MUTABLE, SEM::Type::LVALUE, objectType);
							std::vector<llvm::Type*> types;
							types.push_back(genFunctionType(type->methodType.functionType, getTypeInstancePointer(type->methodType.objectType))->getPointerTo());
							types.push_back(genType(pointerToObjectType));
							return llvm::StructType::get(llvm::getGlobalContext(), types);
						}
						default: {
							assert(false && "Unknown type enum for generating type");
							return llvm::Type::getVoidTy(llvm::getGlobalContext());
						}
					}
				}
				
				llvm::Value* genSizeOf(SEM::Type* type) {
					const size_t sizeTypeWidth = targetInfo_.getPrimitiveSize("size_t");
					
					switch(type->typeEnum) {
						case SEM::Type::VOID:
						case SEM::Type::NULLT: {
							// Void and null have zero size.
							return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(sizeTypeWidth, 0));
						}
						case SEM::Type::NAMED: {
							return builder_.CreateCall(genSizeOfMethod(type->namedType.typeInstance));
						}
						case SEM::Type::POINTER:
						case SEM::Type::FUNCTION: {
							return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(sizeTypeWidth, targetInfo_.getPointerSize() / 8));
						}
						case SEM::Type::METHOD: {
							return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(sizeTypeWidth, 2 * targetInfo_.getPointerSize() / 8));
						}
						default: {
							assert(false && "Unknown type enum for generating sizeof");
							return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(sizeTypeWidth, 0));
						}
					}
				}
				
				llvm::Value* genAlloca(SEM::Type* type) {
					llvm::Type* rawType = genType(type);
					
					switch(type->typeEnum) {
						case SEM::Type::VOID:
						case SEM::Type::NULLT:
						case SEM::Type::POINTER:
						case SEM::Type::FUNCTION:
						case SEM::Type::METHOD: {
							return builder_.CreateAlloca(rawType);
						}
						case SEM::Type::NAMED: {
							SEM::TypeInstance* typeInstance = type->namedType.typeInstance;
							
							if(typeInstance->isPrimitive() || typeInstance->isDefinition()) {
								return builder_.CreateAlloca(rawType);
							} else {
								llvm::Value* alloca = builder_.CreateAlloca(llvm::Type::getInt8Ty(llvm::getGlobalContext()), genSizeOf(type));
								return builder_.CreatePointerCast(alloca, genStructType(typeInstance)->getPointerTo());
							}
						}
						default: {
							assert(false && "Unknown type enum for generating alloca");
							return NULL;
						}
					}
				}
				
				llvm::Value* genStore(llvm::Value* value, llvm::Value* var, SEM::Type* type) {
					switch(type->typeEnum) {
						case SEM::Type::VOID:
						case SEM::Type::NULLT:
						case SEM::Type::POINTER:
						case SEM::Type::FUNCTION:
						case SEM::Type::METHOD: {
							return builder_.CreateStore(value, var);
						}
						case SEM::Type::NAMED: {
							SEM::TypeInstance* typeInstance = type->namedType.typeInstance;
							
							if(typeInstance->isPrimitive() || typeInstance->isStruct()) {
								return builder_.CreateStore(value, var);
							} else {
								if(typeInstance->isDefinition()) {
									return builder_.CreateStore(builder_.CreateLoad(value), var);
								} else {
									return builder_.CreateMemCpy(var, value, genSizeOf(type), 1);
								}
							}
						}
						default: {
							assert(false && "Unknown type enum for generating store");
							return NULL;
						}
					}
				}
				
				llvm::Value* genLoad(llvm::Value* var, SEM::Type* type) {
					switch(type->typeEnum) {
						case SEM::Type::VOID:
						case SEM::Type::NULLT:
						case SEM::Type::POINTER:
						case SEM::Type::FUNCTION:
						case SEM::Type::METHOD: {
							return builder_.CreateLoad(var);
						}
						case SEM::Type::NAMED: {
							SEM::TypeInstance* typeInstance = type->namedType.typeInstance;
							
							if(typeInstance->isPrimitive() || typeInstance->isStruct()) {
								return builder_.CreateLoad(var);
							} else {
								return var;
							}
						}
						default: {
							assert(false && "Unknown type enum for generating load");
							return NULL;
						}
					}
				}
				
				llvm::Function* genFunctionDef(SEM::Function* function) {
					assert(function != NULL && "Generating a function definition requires a non-NULL SEM Function object");
					
					if(function->scope == NULL) return NULL;
					
					currentFunction_ = genFunctionDecl(function);
					assert(currentFunction_ != NULL && "Generating a function definition requires a valid declaration");
					currentBasicBlock_ = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", currentFunction_);
					builder_.SetInsertPoint(currentBasicBlock_);
					// Store arguments onto stack.
					llvm::Function::arg_iterator arg = currentFunction_->arg_begin();
					SEM::Type* returnType = function->type->functionType.returnType;
					
					if(returnType->isClass()) {
						returnVar_ = arg++;
					} else {
						returnVar_ = NULL;
					}
					
					if(function->isMethod) {
						// Generating a method, so capture the 'this' pointer.
						thisPointer_ = arg++;
					}
					
					const std::vector<SEM::Var*>& parameterVars = function->parameters;
					
					for(std::size_t i = 0; i < parameterVars.size(); ++arg, i++) {
						SEM::Var* paramVar = parameterVars.at(i);
						// Create an alloca for this variable.
						llvm::Value* stackObject = genAlloca(paramVar->type);
						assert(paramVar->id == paramVariables_.size()
							   && "Parameter variables' ids should match their position in the parameter variable array");
						paramVariables_.push_back(stackObject);
						// Store the initial value into the alloca.
						genStore(arg, stackObject, paramVar->type);
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
					returnVar_ = NULL;
					thisPointer_ = NULL;
					return currentFunction_;
				}
				
				void genScope(SEM::Scope* scope) {
					for(std::size_t i = 0; i < scope->localVariables.size(); i++) {
						SEM::Var* localVar = scope->localVariables.at(i);
						// Create an alloca for this variable.
						llvm::Value* stackObject = genAlloca(localVar->type);
						assert(localVar->id == localVariables_.size()
							   && "Local variables' ids should match their position in the local variable array");
						localVariables_.push_back(stackObject);
					}
					
					for(std::size_t i = 0; i < scope->statementList.size(); i++) {
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
							genStore(genValue(rValue), genValue(lValue, true), rValue->type);
							break;
						}
						case SEM::Statement::RETURN: {
							if(statement->returnStmt.value != NULL && statement->returnStmt.value->type->typeEnum != SEM::Type::VOID) {
								llvm::Value* returnValue = genValue(statement->returnStmt.value);
								
								if(returnVar_ != NULL) {
									genStore(returnValue, returnVar_, statement->returnStmt.value->type);
									builder_.CreateRetVoid();
								} else {
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
				
				llvm::Value* generateLValue(SEM::Value* value) {
					if(value->type->isLValue) {
						return genValue(value, true);
					} else {
						llvm::Value* lValue = genAlloca(value->type);
						llvm::Value* rValue = genValue(value);
						genStore(rValue, lValue, value->type);
						return lValue;
					}
				}
				
				llvm::Value* genValue(SEM::Value* value, bool genLValue = false) {
					assert(value != NULL && "Cannot generate NULL value");
					
					LOG(LOG_INFO, "Generating value %s.",
						value->toString().c_str());
					
					switch(value->typeEnum) {
						case SEM::Value::CONSTANT: {
							switch(value->constant->getType()) {
								case Locic::Constant::NULLVAL:
									return llvm::ConstantPointerNull::get(llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(llvm::getGlobalContext())));
								case Locic::Constant::BOOLEAN:
									return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(1, value->constant->getBool()));
								case Locic::Constant::SIGNEDINT: {
									const std::size_t primitiveSize = targetInfo_.getPrimitiveSize(value->constant->getTypeName());
									return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(primitiveSize, value->constant->getInt()));
								}
								case Locic::Constant::UNSIGNEDINT: {
									const std::size_t primitiveSize = targetInfo_.getPrimitiveSize(value->constant->getTypeName());
									return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(primitiveSize, value->constant->getUint()));
								}
								case Locic::Constant::FLOATINGPOINT: {
									switch(value->constant->getFloatType()) {
										case Locic::Constant::FLOAT:
											return llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat((float) value->constant->getFloat()));
										case Locic::Constant::DOUBLE:
											return llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat((double) value->constant->getFloat()));
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
									
									switch(value->constant->getStringType()) {
										case Locic::Constant::CSTRING: {
											const bool isConstant = true;
											llvm::ArrayType* arrayType = llvm::ArrayType::get(llvm::Type::getInt8Ty(llvm::getGlobalContext()), stringValue.size() + 1);
											llvm::Constant* constArray = llvm::ConstantDataArray::getString(llvm::getGlobalContext(), stringValue.c_str());
											llvm::GlobalVariable* globalArray = new llvm::GlobalVariable(*module_, arrayType, isConstant, llvm::GlobalValue::PrivateLinkage, constArray, "");
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
							return genValue(value->copyValue.value);
						}
						case SEM::Value::VAR: {
							SEM::Var* var = value->varValue.var;
							
							switch(var->typeEnum) {
								case SEM::Var::PARAM: {
									llvm::Value* val = paramVariables_.at(var->id);
									assert(val != NULL && "Parameter variable must exist to be referenced");
									
									if(genLValue) {
										return val;
									} else {
										return genLoad(val, value->type);
									}
								}
								case SEM::Var::LOCAL: {
									llvm::Value* val = localVariables_.at(var->id);
									assert(val != NULL && "Local variable must exist to be referenced");
									
									if(genLValue) {
										return val;
									} else {
										return genLoad(val, value->type);
									}
								}
								case SEM::Var::MEMBER: {
									assert(thisPointer_ != NULL && "The 'this' pointer cannot be null when accessing member variables");
									llvm::Value* memberPtr = builder_.CreateConstInBoundsGEP2_32(thisPointer_, 0, var->id);
									
									if(genLValue) {
										return memberPtr;
									} else {
										return genLoad(memberPtr, value->type);
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
								return genLoad(genValue(value->deref.value), value->type);
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
									
							if(destType->typeEnum == SEM::Type::VOID) {
								// All casts to void have the same outcome.
								return llvm::UndefValue::get(llvm::Type::getVoidTy(llvm::getGlobalContext()));
							}
							
							switch(sourceType->typeEnum) {
								case SEM::Type::VOID: {
									return codeValue;
								}
								case SEM::Type::NULLT: {
									switch(destType->typeEnum) {
										case SEM::Type::NULLT:
											return codeValue;
										case SEM::Type::POINTER:
										case SEM::Type::FUNCTION:
											return builder_.CreatePointerCast(codeValue, genType(destType));
										case SEM::Type::NAMED: {
											assert(false && "TODO");
											return NULL;
										}
										default: {
											assert(false && "Invalid cast from null");
											return llvm::UndefValue::get(llvm::Type::getVoidTy(llvm::getGlobalContext()));
										}
									}
								}
								case SEM::Type::NAMED: {
									if(sourceType->namedType.typeInstance == destType->namedType.typeInstance) {
										return codeValue;
									}
									
									assert(false && "Casts between named types not implemented");
									return NULL;
								}
								case SEM::Type::POINTER: {
									if(genLValue) {
										return builder_.CreatePointerCast(codeValue, genType(destType)->getPointerTo());
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
						case SEM::Value::POLYCAST: {
							assert(!genLValue && "Cannot generate interfaces as lvalues in polycast");
							llvm::Value* rawValue = genValue(value->polyCast.value);
							SEM::Type* sourceType = value->polyCast.value->type;
							SEM::Type* destType = value->type;
							assert(sourceType->isPointer() && "Polycast source type must be pointer");
							assert(destType->isPointer() && "Polycast dest type must be pointer");
							SEM::Type* sourceTarget = sourceType->getPointerTarget();
							SEM::Type* destTarget = destType->getPointerTarget();
							assert(destTarget->isInterface() && "Polycast dest target type must be interface");
							
							if(sourceTarget->isInterface()) {
								// Get the object pointer.
								llvm::Value* objectPointerValue = builder_.CreateExtractValue(rawValue, std::vector<unsigned>(1, 0));
								// Cast it as a pointer to the opaque struct representing
								// destination interface type.
								llvm::Value* objectPointer = builder_.CreatePointerCast(objectPointerValue,
															 genStructType(destTarget->getObjectType())->getPointerTo());
								// Get the vtable pointer.
								llvm::Value* vtablePointer = builder_.CreateExtractValue(rawValue, std::vector<unsigned>(1, 1));
								// Build the new interface pointer struct with these values.
								llvm::Value* interfaceValue = llvm::UndefValue::get(genType(destType));
								interfaceValue = builder_.CreateInsertValue(interfaceValue, objectPointer, std::vector<unsigned>(1, 0));
								interfaceValue = builder_.CreateInsertValue(interfaceValue, vtablePointer, std::vector<unsigned>(1, 1));
								return interfaceValue;
							} else if(sourceTarget->isClass()) {
								// Cast class pointer to pointer to the opaque struct
								// representing destination interface type.
								llvm::Value* objectPointer = builder_.CreatePointerCast(rawValue,
															 genStructType(destTarget->getObjectType())->getPointerTo());
								// Get the vtable pointer.
								llvm::Value* vtablePointer = genVTable(sourceTarget->getObjectType());
								// Build the new interface pointer struct with these values.
								llvm::Value* interfaceValue = llvm::UndefValue::get(genType(destType));
								interfaceValue = builder_.CreateInsertValue(interfaceValue, objectPointer, std::vector<unsigned>(1, 0));
								interfaceValue = builder_.CreateInsertValue(interfaceValue, vtablePointer, std::vector<unsigned>(1, 1));
								return interfaceValue;
							} else {
								assert(false && "Polycast source target type must be class or interface");
								return NULL;
							}
						}
						case SEM::Value::INTERNALCONSTRUCT: {
							const std::vector<SEM::Value*>& parameters = value->internalConstruct.parameters;
							llvm::Value* objectValue = genAlloca(value->type);
							
							for(size_t i = 0; i < parameters.size(); i++) {
								SEM::Value* paramValue = parameters.at(i);
								genStore(genValue(paramValue), builder_.CreateConstInBoundsGEP2_32(objectValue, 0, i), paramValue->type);
							}
							
							return objectValue;
						}
						case SEM::Value::MEMBERACCESS: {
							if(genLValue) {
								return builder_.CreateConstInBoundsGEP2_32(genValue(value->memberAccess.object, true), 0, value->memberAccess.memberId);
							} else {
								return builder_.CreateExtractValue(genValue(value->memberAccess.object), std::vector<unsigned>(1, value->memberAccess.memberId));
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
							SEM::Type* returnType = value->type;
							llvm::Value* returnValue = NULL;
							
							if(returnType->isClass()) {
								returnValue = genAlloca(returnType);
								assert(returnValue != NULL && "Must have lvalue for holding class return value so it can be passed by reference");
								parameters.push_back(returnValue);
							}
							
							for(std::size_t i = 0; i < paramList.size(); i++) {
								llvm::Value* argValue = genValue(paramList.at(i));
								
								// When calling var-args functions, all 'char' and
								// 'short' values must be extended to 'int' values,
								// and all 'float' values must be converted to 'double'
								// values.
								if(functionType->isFunctionVarArg()) {
									llvm::Type* argType = argValue->getType();
									const unsigned sizeInBits = argType->getPrimitiveSizeInBits();
									
									if(argType->isIntegerTy() && sizeInBits < targetInfo_.getPrimitiveSize("int")) {
										// Need to extend to int.
										// TODO: this doesn't handle unsigned types; perhaps
										// this code should be moved to semantic analysis.
										argValue = builder_.CreateSExt(argValue, llvm::IntegerType::get(llvm::getGlobalContext(), targetInfo_.getPrimitiveSize("int")));
									} else if(argType->isFloatingPointTy() && sizeInBits < 64) {
										// Need to extend to double.
										argValue = builder_.CreateFPExt(argValue, llvm::Type::getDoubleTy(llvm::getGlobalContext()));
									}
								}
								
								parameters.push_back(argValue);
							}
							
							llvm::Value* callReturnValue = builder_.CreateCall(function, parameters);
							
							if(returnValue != NULL) {
								return genLoad(returnValue, returnType);
							} else {
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
							llvm::Value* methodValue = llvm::UndefValue::get(genType(value->type));
							methodValue = builder_.CreateInsertValue(methodValue, function, std::vector<unsigned>(1, 0));
							methodValue = builder_.CreateInsertValue(methodValue, dataPointer, std::vector<unsigned>(1, 1));
							return methodValue;
						}
						case SEM::Value::METHODCALL: {
							LOG(LOG_EXCESSIVE, "Generating method call value %s.",
								value->methodCall.methodValue->toString().c_str());
							
							llvm::Value* method = genValue(value->methodCall.methodValue);
							llvm::Value* function = builder_.CreateExtractValue(method, std::vector<unsigned>(1, 0));
							llvm::Value* dataPointer = builder_.CreateExtractValue(method, std::vector<unsigned>(1, 1));
							
							std::vector<llvm::Value*> parameters;
							
							SEM::Type* returnType = value->type;
							llvm::Value* returnValue = NULL;
							
							if(returnType->isClass()) {
								returnValue = genAlloca(returnType);
								assert(returnValue != NULL && "Must have lvalue for holding class return value so it can be passed by reference");
								parameters.push_back(returnValue);
							}
							
							parameters.push_back(dataPointer);
							const std::vector<SEM::Value*>& paramList = value->methodCall.parameters;
							
							for(std::size_t i = 0; i < paramList.size(); i++) {
								LOG(LOG_EXCESSIVE, "Generating method call argument %s.",
									paramList.at(i)->toString().c_str());
								parameters.push_back(genValue(paramList.at(i)));
							}
							
							LOG(LOG_EXCESSIVE, "Creating call.");
							
							llvm::Value* callReturnValue = builder_.CreateCall(function, parameters);
							
							if(returnValue != NULL) {
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
			codeGen_->genNamespace(nameSpace);
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

