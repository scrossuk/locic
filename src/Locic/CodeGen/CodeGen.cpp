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
#include <Locic/CodeGen/Primitives.hpp>
#include <Locic/CodeGen/Support.hpp>
#include <Locic/CodeGen/TargetInfo.hpp>
#include <Locic/CodeGen/VTable.hpp>

namespace Locic {

	namespace CodeGen {
	
		Map<MethodHash, SEM::Function*> CreateFunctionHashMap(SEM::TypeInstance* typeInstance) {
			Map<MethodHash, SEM::Function*> hashMap;
			
			const std::vector<SEM::Function*>& functions = typeInstance->functions();
			
			for(size_t i = 0; i < functions.size(); i++){
				SEM::Function* function = functions.at(i);
				hashMap.insert(CreateMethodNameHash(function->name()), function);
			}
			
			return hashMap;
		}
		
		std::vector<MethodHash> CreateHashArray(const Map<MethodHash, SEM::Function*>& hashMap) {
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
				
				Map<SEM::TypeInstance*, llvm::StructType*> typeInstances_;
				Map<SEM::TypeInstance*, llvm::Function*> sizeOfMethods_;
				Map<SEM::TypeInstance*, llvm::GlobalVariable*> vtables_;
				
				Map<SEM::Function*, llvm::Function*> functions_;
				Map<SEM::Var *, size_t> memberVarIds_;
				Map<SEM::Var *, llvm::Value *> localVariables_, paramVariables_;
				llvm::Value* returnVar_;
				llvm::Value* thisPointer_;
				
			public:
				InternalCodeGen(const TargetInfo& targetInfo, const std::string& moduleName)
					: name_(moduleName),
					  targetInfo_(targetInfo),
					  module_(new llvm::Module(name_.c_str(), llvm::getGlobalContext())),
					  builder_(llvm::getGlobalContext()),
					  returnVar_(NULL),
					  thisPointer_(NULL) {
					module_->setTargetTriple(targetInfo_.getTargetTriple());
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
				
				llvm::GlobalValue::LinkageTypes getFunctionLinkage(SEM::TypeInstance* type){
					if(type == NULL){
						return llvm::Function::ExternalLinkage;
					}
					
					return type->isClass()
						? llvm::Function::ExternalLinkage
						: llvm::Function::LinkOnceODRLinkage;
				}
				
				// ---- Pass 1: Generate type placeholders.
				
				void genTypeInstanceTypePlaceholders(const Name& name, SEM::TypeInstance* typeInstance){
					assert(typeInstance != NULL);
					
					if(typeInstance->isPrimitive()){
						// Skip.
						return;
					}
					
					const Name typeName = name + typeInstance->name();
					
					llvm::StructType* structType = llvm::StructType::create(llvm::getGlobalContext(),
						typeName.genString());
					
					typeInstances_.insert(typeInstance, structType);
				}
				
				void genNamespaceTypePlaceholders(const Name& name, SEM::Namespace* nameSpace){
					const Name nsName = name + nameSpace->name();
					
					const std::vector<SEM::Namespace*>& namespaces = nameSpace->namespaces();
					for(size_t i = 0; i < namespaces.size(); i++){
						genNamespaceTypePlaceholders(nsName, namespaces.at(i));
					}
					
					const std::vector<SEM::TypeInstance*>& typeInstances = nameSpace->typeInstances();
					for(size_t i = 0; i < typeInstances.size(); i++){
						genTypeInstanceTypePlaceholders(nsName, typeInstances.at(i));
					}
				}
				
				// ---- Pass 2: Generate type members.
				
				void genTypeInstanceTypeMembers(const Name& name, SEM::TypeInstance* typeInstance){
					assert(typeInstance != NULL);
					
					if(typeInstance->isPrimitive()){
						// Skip.
						return;
					}
					
					const Name typeName = name + typeInstance->name();
					
					// Generate type member variables.
					llvm::StructType* structType = typeInstances_.get(typeInstance);
					
					if(typeInstance->isClassDef() || typeInstance->isStructDef()) {
						// Generating the type for a class or struct definition, so
						// the size and contents of the type instance is known and
						// hence the contents can be specified.
						std::vector<llvm::Type*> memberVariables;
						
						const std::vector<SEM::Var*>& variables = typeInstance->variables();
						
						for(size_t i = 0; i < variables.size(); i++){
							SEM::Var* var = variables.at(i);
							memberVariables.push_back(genType(var->type()));
							memberVarIds_.insert(var, i);
						}
						
						LOG(LOG_INFO, "Set %llu member variables for type '%s'.",
							(unsigned long long) memberVariables.size(), typeInstance->name().c_str());
						
						structType->setBody(memberVariables);
					}
				}
				
				void genNamespaceTypeMembers(const Name& name, SEM::Namespace* nameSpace){
					const Name nsName = name + nameSpace->name();
					
					const std::vector<SEM::Namespace*>& namespaces = nameSpace->namespaces();
					for(size_t i = 0; i < namespaces.size(); i++){
						genNamespaceTypeMembers(nsName, namespaces.at(i));
					}
					
					const std::vector<SEM::TypeInstance*>& typeInstances = nameSpace->typeInstances();
					for(size_t i = 0; i < typeInstances.size(); i++){
						genTypeInstanceTypeMembers(nsName, typeInstances.at(i));
					}
				}
				
				// ---- Pass 3: Generate function declarations.
				
				// Generate 'sizeof()' method decl.
				void genSizeOfMethodDecl(const Name& typeName, SEM::TypeInstance* typeInstance) {
					const std::string functionName = std::string("__BUILTIN__") + typeName.genString() + "__sizeof";
					
					const size_t sizeTypeWidth = targetInfo_.getPrimitiveSize("size_t");
					llvm::Type* sizeType = llvm::IntegerType::get(llvm::getGlobalContext(), sizeTypeWidth);
					llvm::FunctionType* functionType = llvm::FunctionType::get(sizeType, std::vector<llvm::Type*>(), false);
					
					llvm::Function* function = llvm::Function::Create(functionType, getFunctionLinkage(typeInstance), functionName, module_);
					function->setDoesNotAccessMemory();
					
					sizeOfMethods_.insert(typeInstance, function);
				}
				
				void genFunctionDecl(const Name& name, SEM::TypeInstance * parent, SEM::Function* function){
					if(function->isMethod()) assert(parent != NULL);
					
					const std::string functionName = (name + function->name()).genString();
					
					llvm::Type* thisType = function->isMethod() ? getTypeInstancePointer(*parent)
						: NULL;
					
					llvm::Function* functionDecl = llvm::Function::Create(genFunctionType(function->type(), thisType),
						getFunctionLinkage(parent), functionName, module_);
					
					if(function->type()->getFunctionReturnType()->isClassOrTemplateVar()) {
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
					
					functions_.insert(function, functionDecl);
				}
				
				void genTypeInstanceFunctionDecls(const Name& name, SEM::TypeInstance* typeInstance){
					const Name typeName = name + typeInstance->name();
					
					genSizeOfMethodDecl(typeName, typeInstance);
					
					const std::vector<SEM::Function*>& functions = typeInstance->functions();
					for(size_t i = 0; i < functions.size(); i++){
						genFunctionDecl(typeName, typeInstance, functions.at(i));
					}
				}
				
				void genNamespaceFunctionDecls(const Name& name, SEM::Namespace* nameSpace){
					const Name nsName = name + nameSpace->name();
					
					const std::vector<SEM::Namespace*>& namespaces = nameSpace->namespaces();
					for(size_t i = 0; i < namespaces.size(); i++){
						genNamespaceFunctionDecls(nsName, namespaces.at(i));
					}
					
					const std::vector<SEM::TypeInstance*>& typeInstances = nameSpace->typeInstances();
					for(size_t i = 0; i < typeInstances.size(); i++){
						genTypeInstanceFunctionDecls(nsName, typeInstances.at(i));
					}
					
					const std::vector<SEM::Function*>& functions = nameSpace->functions();
					for(size_t i = 0; i < functions.size(); i++){
						genFunctionDecl(nsName, NULL, functions.at(i));
					}
				}
				
				// ---- Pass 4: Generate vtables.
				
				void genTypeInstanceVTables(const Name& name, SEM::TypeInstance* typeInstance){
					assert(typeInstance != NULL);
					
					const Name typeName = name + typeInstance->name();
					
					const std::string vtableName = makeString("__VTABLE__%s", typeName.genString().c_str());
					
					const bool isConstant = true;
					llvm::GlobalVariable* globalVariable = new llvm::GlobalVariable(*module_, getVTableType(),
							isConstant, getFunctionLinkage(typeInstance), NULL, vtableName);
					
					if(typeInstance->isClassDecl()) return;
					
					// Generate the vtable.
					const Map<MethodHash, SEM::Function*> functionHashMap = CreateFunctionHashMap(typeInstance);
					std::vector<MethodHash> hashArray = CreateHashArray(functionHashMap);
					
					const VirtualTable virtualTable = VirtualTable::CalculateFromHashes(hashArray);
					
					std::vector<llvm::Constant*> vtableStructElements;
					
					// Destructor.
					const bool isVarArg = false;
					llvm::PointerType* destructorType = llvm::FunctionType::get(voidType(),
						std::vector<llvm::Type*>(1, i8PtrType()), isVarArg)->getPointerTo();
					vtableStructElements.push_back(llvm::ConstantPointerNull::get(destructorType));
					
					// Method slots.
					std::vector<llvm::Constant*> methodSlotElements;
					
					for(size_t i = 0; i < VTABLE_SIZE; i++) {
						const std::list<MethodHash>& slotList = virtualTable.table().at(i);
						
						if(slotList.empty()) {
							methodSlotElements.push_back(llvm::ConstantPointerNull::get(i8PtrType()));
						} else if(slotList.size() > 1) {
							LOG(LOG_ERROR, "COLLISION at %llu for type %s.\n",
								   (unsigned long long) i, typeInstance->toString().c_str());
							//assert(false && "Collision resolution not implemented.");
							methodSlotElements.push_back(llvm::ConstantPointerNull::get(i8PtrType()));
						} else {
							assert(slotList.size() == 1);
							SEM::Function* semFunction = functionHashMap.get(slotList.front());
							llvm::Function* function = functions_.get(semFunction);
							methodSlotElements.push_back(llvm::ConstantExpr::getPointerCast(function, i8PtrType()));
						}
					}
					
					llvm::Constant* methodSlotTable = llvm::ConstantArray::get(
						llvm::ArrayType::get(i8PtrType(), VTABLE_SIZE), methodSlotElements);
					vtableStructElements.push_back(methodSlotTable);
					
					llvm::Constant* vtableStruct =
						llvm::ConstantStruct::get(getVTableType(), vtableStructElements);
						
					globalVariable->setInitializer(vtableStruct);
					
					vtables_.insert(typeInstance, globalVariable);
				}
				
				void genNamespaceVTables(const Name& name, SEM::Namespace* nameSpace){
					const Name nsName = name + nameSpace->name();
					
					const std::vector<SEM::Namespace*>& namespaces = nameSpace->namespaces();
					for(size_t i = 0; i < namespaces.size(); i++){
						genNamespaceVTables(nsName, namespaces.at(i));
					}
					
					const std::vector<SEM::TypeInstance*>& typeInstances = nameSpace->typeInstances();
					for(size_t i = 0; i < typeInstances.size(); i++){
						genTypeInstanceVTables(nsName, typeInstances.at(i));
					}
				}
				
				// ---- Pass 5: Generate function code.
				
				// Generate 'sizeof()' method code.
				void genSizeOfMethodDef(SEM::TypeInstance* typeInstance) {
					llvm::Function* function = sizeOfMethods_.get(typeInstance);
					
					if(typeInstance->isPrimitive()){
						createPrimitiveSizeOf(*module_, typeInstance->name(), function);
					}else{
						assert(!typeInstance->isDeclaration());
						
						const size_t sizeTypeWidth = targetInfo_.getPrimitiveSize("size_t");
						
						llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
						builder_.SetInsertPoint(basicBlock);
						llvm::Value* zero = llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(sizeTypeWidth, 0));
						llvm::Value* one = llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(sizeTypeWidth, 1));
						llvm::Value* classSize = zero;
						
						// Add up all member variable sizes.
						const std::vector<SEM::Var *>& variables = typeInstance->variables();
						
						for(size_t i = 0; i < variables.size(); i++){
							SEM::Var * var = variables.at(i);
							classSize = builder_.CreateAdd(classSize, genSizeOf(var->type()));
						}
						
						// Class sizes must be at least one byte.
						llvm::Value* isZero = builder_.CreateICmpEQ(classSize, zero);
						classSize = builder_.CreateSelect(isZero, one, classSize);
						builder_.CreateRet(classSize);
					}
				}
				
				void genInterfaceMethod(SEM::Function* function) {
					assert(function->isDeclaration() && "Interface methods must be declarations");
					assert(function->isMethod() && "Must be a method");
					
					llvm::Function* generatedFunction = functions_.get(function);
					assert(generatedFunction != NULL);
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", generatedFunction);
					builder_.SetInsertPoint(basicBlock);
					
					// Store arguments onto stack.
					llvm::Function::arg_iterator arg = generatedFunction->arg_begin();
					SEM::Type* returnType = function->type()->getFunctionReturnType();
					llvm::Value* returnVar = returnType->isClass() ? arg++ : NULL;
					
					// Get the 'this' record, which is the
					// pair of the 'this' pointer and the
					// method vtable pointer.
					llvm::Value* thisRecord = arg++;
					
					// Get the 'this' pointer.
					llvm::Value* thisPointer = builder_.CreateExtractValue(thisRecord, std::vector<unsigned>(1, 0), "thisPointer");
					
					// Get the vtable pointer.
					llvm::Value* vtablePointer = builder_.CreateExtractValue(thisRecord,
						std::vector<unsigned>(1, 1), "vtablePointer");
					
					const Locic::CodeGen::MethodHash methodHash = Locic::CodeGen::CreateMethodNameHash(function->name());
					const size_t offset = methodHash % VTABLE_SIZE;
					
					std::vector<llvm::Value*> vtableEntryGEP;
					vtableEntryGEP.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, 0)));
					vtableEntryGEP.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, 1)));
					vtableEntryGEP.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, offset)));
					
					llvm::Value* vtableEntryPointer = builder_.CreateInBoundsGEP(vtablePointer, vtableEntryGEP, "vtableEntryPointer");
					llvm::Value* methodFunctionPointer = builder_.CreateLoad(vtableEntryPointer, "methodFunctionPointer");
					llvm::Type* methodFunctionType = genFunctionType(function->type(), thisPointer->getType());
					llvm::Value* castedMethodFunctionPointer = builder_.CreatePointerCast(
								methodFunctionPointer, methodFunctionType->getPointerTo(), "castedMethodFunctionPointer");
					std::vector<llvm::Value*> arguments;
					
					if(returnVar != NULL) arguments.push_back(returnVar);
					
					arguments.push_back(thisPointer);
					
					while(arg != generatedFunction->arg_end()) arguments.push_back(arg++);
					
					llvm::FunctionType* asmFunctionType = llvm::FunctionType::get(voidType(), std::vector<llvm::Type*>(), false);
					const std::string assembly = makeString("movl $$%llu, %%eax",
						(unsigned long long) methodHash);
													  
					llvm::InlineAsm* setEax = llvm::InlineAsm::get(asmFunctionType, assembly, "~eax", true);
					builder_.CreateCall(setEax);
					
					llvm::Value* methodCallValue = builder_.CreateCall(castedMethodFunctionPointer,
						arguments, returnType->isVoid() ? "" : "methodCallValue");
												   
					if(returnType->isVoid()) {
						builder_.CreateRetVoid();
					} else {
						builder_.CreateRet(methodCallValue);
					}
					
					// Check the generated function is correct.
					verifyFunction(*generatedFunction);
				}
				
				void genFunctionDef(SEM::Function* function){
					assert(function != NULL && "Generating a function definition requires a non-NULL SEM Function object");
					
					if(function->isDeclaration()) return;
					
					currentFunction_ = functions_.get(function);
					assert(currentFunction_ != NULL);
					
					currentBasicBlock_ = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", currentFunction_);
					
					builder_.SetInsertPoint(currentBasicBlock_);
					
					// Store arguments onto stack.
					llvm::Function::arg_iterator arg = currentFunction_->arg_begin();
					SEM::Type* returnType = function->type()->getFunctionReturnType();
					
					if(returnType->isClass()) {
						returnVar_ = arg++;
					} else {
						returnVar_ = NULL;
					}
					
					if(function->isMethod()) {
						// Generating a method, so capture the 'this' pointer.
						thisPointer_ = arg++;
					}
					
					assert(paramVariables_.empty());
					assert(localVariables_.empty());
					
					const std::vector<SEM::Var*>& parameterVars = function->parameters();
					
					for(std::size_t i = 0; i < parameterVars.size(); ++arg, i++) {
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
					
					// Check the generated function is correct.
					verifyFunction(*currentFunction_);
					paramVariables_.clear();
					localVariables_.clear();
					returnVar_ = NULL;
					thisPointer_ = NULL;
				}
				
				void genTypeInstanceFunctionDefs(SEM::TypeInstance* typeInstance){
					if(typeInstance->isPrimitive() || typeInstance->isDefinition()){
						genSizeOfMethodDef(typeInstance);
					}
					
					if(typeInstance->isClass()){
						const std::vector<SEM::Function*>& functions = typeInstance->functions();
						for(size_t i = 0; i < functions.size(); i++){
							genFunctionDef(functions.at(i));
						}
					}else if(typeInstance->isPrimitive()) {
						const std::vector<SEM::Function*>& functions = typeInstance->functions();
						for(size_t i = 0; i < functions.size(); i++){
							SEM::Function* function = functions.at(i);
							createPrimitiveMethod(*module_, typeInstance->name(),
								function->name(), functions_.get(function));
						}
					}else if(typeInstance->isInterface()) {
						const std::vector<SEM::Function *>& functions = typeInstance->functions();
						for(size_t i = 0; i < functions.size(); i++){
							genInterfaceMethod(functions.at(i));
						}
					}
				}
				
				void genNamespaceFunctionDefs(SEM::Namespace* nameSpace) {
					const std::vector<SEM::Namespace*>& namespaces = nameSpace->namespaces();
					for(size_t i = 0; i < namespaces.size(); i++){
						genNamespaceFunctionDefs(namespaces.at(i));
					}
					
					const std::vector<SEM::TypeInstance*>& typeInstances = nameSpace->typeInstances();
					for(size_t i = 0; i < typeInstances.size(); i++){
						genTypeInstanceFunctionDefs(typeInstances.at(i));
					}
					
					const std::vector<SEM::Function*>& functions = nameSpace->functions();
					for(size_t i = 0; i < functions.size(); i++){
						genFunctionDef(functions.at(i));
					}
				}
				
				// ---- Function code generation.
				
				llvm::FunctionType* genFunctionType(SEM::Type* type, llvm::Type* thisPointerType = NULL) {
					assert(type != NULL && "Generating a function type requires a non-NULL SEM Type object");
					assert(type->isFunction() && "Type must be a function type for it to be generated as such");
					SEM::Type* semReturnType = type->getFunctionReturnType();
					assert(semReturnType != NULL && "Generating function return type requires a non-NULL SEM return type");
					llvm::Type* returnType = genType(semReturnType);
					std::vector<llvm::Type*> paramTypes;
					
					if(semReturnType->isClassOrTemplateVar()) {
						// Class return values are constructed on the caller's
						// stack, and given to the callee as a pointer.
						paramTypes.push_back(returnType->getPointerTo());
						returnType = llvm::Type::getVoidTy(llvm::getGlobalContext());
					}
					
					if(thisPointerType != NULL) {
						// Generating a method, so add the 'this' pointer.
						paramTypes.push_back(thisPointerType);
					}
					
					const std::vector<SEM::Type*>& params = type->getFunctionParameterTypes();
					
					for(std::size_t i = 0; i < params.size(); i++) {
						SEM::Type* paramType = params.at(i);
						llvm::Type* rawType = genType(paramType);
						
						if(paramType->isObject()) {
							SEM::TypeInstance* typeInstance = paramType->getObjectType();
							
							if(typeInstance->isClass()) {
								rawType = rawType->getPointerTo();
							}
						}
						
						paramTypes.push_back(rawType);
					}
					
					return llvm::FunctionType::get(returnType, paramTypes, type->isFunctionVarArg());
				}
				
				llvm::Type* genObjectType(SEM::Type* type){
					assert(type->isObject());
					SEM::TypeInstance* typeInstance = type->getObjectType();
					
					if(typeInstance->isPrimitive()) {
						return createPrimitiveType(*module_, typeInstance);
					} else {
						assert(!typeInstance->isInterface() && "Interface types must always be converted by pointer");
						return typeInstances_.get(typeInstance);
					}
				}
				
				llvm::Type* genPointerType(SEM::Type* targetType){
					// Interface pointers/references are actually two pointers:
					// one to the class, and one to the class vtable.
					if(targetType->isInterface()) {
						std::vector<llvm::Type*> types;
						// Class pointer.
						types.push_back(typeInstances_.get(targetType->getObjectType())->getPointerTo());
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
				
				llvm::Type* getTypeInstancePointer(SEM::TypeInstance& typeInstance) {
					return genPointerType(SEM::Type::Object(SEM::Type::MUTABLE,
						SEM::Type::LVALUE, &typeInstance, SEM::Type::NO_TEMPLATE_ARGS));
				}
				
				llvm::Type* genType(SEM::Type* type) {
					switch(type->kind()) {
						case SEM::Type::VOID: {
							return llvm::Type::getVoidTy(llvm::getGlobalContext());
						}
						case SEM::Type::NULLT: {
							return llvm::Type::getInt8Ty(llvm::getGlobalContext())->getPointerTo();
						}
						case SEM::Type::OBJECT: {
							return genObjectType(type);
						}
						case SEM::Type::POINTER: {
							return genPointerType(type->getPointerTarget());
						}
						case SEM::Type::REFERENCE: {
							return genPointerType(type->getReferenceTarget());
						}
						case SEM::Type::FUNCTION: {
							return genFunctionType(type)->getPointerTo();
						}
						case SEM::Type::METHOD: {
							SEM::Type* objectType = type->getMethodObjectType();
							std::vector<llvm::Type*> types;
							types.push_back(genFunctionType(type->getMethodFunctionType(),
								getTypeInstancePointer(*(objectType->getObjectType())))->getPointerTo());
							types.push_back(genPointerType(objectType));
							return llvm::StructType::get(llvm::getGlobalContext(), types);
						}
						case SEM::Type::TEMPLATEVAR: {
							// TODO: There might a more meaningful type to generate here.
							return llvm::Type::getInt8Ty(llvm::getGlobalContext());
						}
						default: {
							assert(false && "Unknown type enum for generating type");
							return llvm::Type::getVoidTy(llvm::getGlobalContext());
						}
					}
				}
				
				llvm::Value* genSizeOf(SEM::Type* type) {
					const size_t sizeTypeWidth = targetInfo_.getPrimitiveSize("size_t");
					
					switch(type->kind()) {
						case SEM::Type::VOID:
						case SEM::Type::NULLT: {
							// Void and null have zero size.
							return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(sizeTypeWidth, 0));
						}
						case SEM::Type::OBJECT: {
							return builder_.CreateCall(sizeOfMethods_.get(type->getObjectType()));
						}
						case SEM::Type::POINTER: {
							const size_t multiplier = type->getPointerTarget()->isInterface() ? 2 : 1;
							return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(sizeTypeWidth,
								multiplier * targetInfo_.getPointerSize() / 8));
						}
						case SEM::Type::REFERENCE: {
							const size_t multiplier = type->getReferenceTarget()->isInterface() ? 2 : 1;
							return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(sizeTypeWidth,
								multiplier * targetInfo_.getPointerSize() / 8));
						}
						case SEM::Type::FUNCTION: {
							return llvm::ConstantInt::get(llvm::getGlobalContext(),
								llvm::APInt(sizeTypeWidth, targetInfo_.getPointerSize() / 8));
						}
						case SEM::Type::METHOD: {
							return llvm::ConstantInt::get(llvm::getGlobalContext(),
								llvm::APInt(sizeTypeWidth, 2 * targetInfo_.getPointerSize() / 8));
						}
						default: {
							assert(false && "Unknown type enum for generating sizeof");
							return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(sizeTypeWidth, 0));
						}
					}
				}
				
				llvm::Value* genAlloca(SEM::Type* type) {
					llvm::Type* rawType = genType(type);
					
					switch(type->kind()) {
						case SEM::Type::VOID:
						case SEM::Type::NULLT:
						case SEM::Type::POINTER:
						case SEM::Type::REFERENCE:
						case SEM::Type::FUNCTION:
						case SEM::Type::METHOD: {
							return builder_.CreateAlloca(rawType);
						}
						case SEM::Type::OBJECT: {
							SEM::TypeInstance* typeInstance = type->getObjectType();
							
							if(typeInstance->isPrimitive() || typeInstance->isDefinition()) {
								return builder_.CreateAlloca(rawType);
							} else {
								llvm::Value* alloca = builder_.CreateAlloca(
									llvm::Type::getInt8Ty(llvm::getGlobalContext()), genSizeOf(type));
								return builder_.CreatePointerCast(alloca,
									typeInstances_.get(typeInstance)->getPointerTo());
							}
						}
						default: {
							assert(false && "Unknown type enum for generating alloca");
							return NULL;
						}
					}
				}
				
				llvm::Value* genStore(llvm::Value* value, llvm::Value* var, SEM::Type* type) {
					switch(type->kind()) {
						case SEM::Type::VOID:
						case SEM::Type::NULLT:
						case SEM::Type::POINTER:
						case SEM::Type::REFERENCE:
						case SEM::Type::FUNCTION:
						case SEM::Type::METHOD: {
							return builder_.CreateStore(value, var);
						}
						case SEM::Type::OBJECT: {
							SEM::TypeInstance* typeInstance = type->getObjectType();
							
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
					switch(type->kind()) {
						case SEM::Type::VOID:
						case SEM::Type::NULLT:
						case SEM::Type::POINTER:
						case SEM::Type::REFERENCE:
						case SEM::Type::FUNCTION:
						case SEM::Type::METHOD: {
							return builder_.CreateLoad(var);
						}
						case SEM::Type::OBJECT: {
							SEM::TypeInstance* typeInstance = type->getObjectType();
							
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
				
				void genScope(const SEM::Scope& scope) {
					for(std::size_t i = 0; i < scope.localVariables().size(); i++) {
						SEM::Var* localVar = scope.localVariables().at(i);
						
						// Create an alloca for this variable.
						llvm::Value* stackObject = genAlloca(localVar->type());
						
						localVariables_.forceInsert(localVar, stackObject);
					}
					
					for(std::size_t i = 0; i < scope.statements().size(); i++) {
						genStatement(scope.statements().at(i));
					}
				}
				
				void genStatement(SEM::Statement* statement) {
					switch(statement->kind()) {
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
							
							if(statement->hasIfFalseScope()) {
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
							if(statement->getReturnValue() != NULL
								&& !statement->getReturnValue()->type()->isVoid()) {
								llvm::Value* returnValue = genValue(statement->getReturnValue());
								
								if(returnVar_ != NULL) {
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
					if(value->type()->isLValue()) {
						return genValue(value, true);
					} else {
						llvm::Value* lValue = genAlloca(value->type());
						llvm::Value* rValue = genValue(value);
						genStore(rValue, lValue, value->type());
						return lValue;
					}
				}
				
				llvm::Value* genValue(SEM::Value* value, bool genLValue = false) {
					assert(value != NULL && "Cannot generate NULL value");
					
					LOG(LOG_INFO, "Generating value %s.",
						value->toString().c_str());
						
					switch(value->kind()) {
						case SEM::Value::CONSTANT: {
							switch(value->constant->getType()) {
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
									switch(value->constant->getFloatType()) {
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
									
									switch(value->constant->getStringType()) {
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
							
							switch(var->kind()) {
								case SEM::Var::PARAM: {
									llvm::Value* val = paramVariables_.get(var);
									
									if(genLValue) {
										return val;
									} else {
										return genLoad(val, value->type());
									}
								}
								case SEM::Var::LOCAL: {
									llvm::Value* val = localVariables_.get(var);
									
									if(genLValue) {
										return val;
									} else {
										return genLoad(val, value->type());
									}
								}
								case SEM::Var::MEMBER: {
									assert(thisPointer_ != NULL &&
										"The 'this' pointer cannot be null when accessing member variables");
									llvm::Value* memberPtr = builder_.CreateConstInBoundsGEP2_32(
										thisPointer_, 0, memberVarIds_.get(var));
									
									if(genLValue) {
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
							if(genLValue) {
								return genValue(value->derefPointer.value);
							} else {
								return genLoad(genValue(value->derefPointer.value), value->type());
							}
						}
						case SEM::Value::REFERENCEOF: {
							return genValue(value->referenceOf.value, true);
						}
						case SEM::Value::DEREF_REFERENCE: {
							if(genLValue) {
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
									
							if(destType->isVoid()) {
								// All casts to void have the same outcome.
								return llvm::UndefValue::get(llvm::Type::getVoidTy(llvm::getGlobalContext()));
							}
							
							switch(sourceType->kind()) {
								case SEM::Type::VOID: {
									return codeValue;
								}
								case SEM::Type::NULLT: {
									switch(destType->kind()) {
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
									if(sourceType->getObjectType() == destType->getObjectType()) {
										return codeValue;
									}
									
									assert(false && "Casts between named types not implemented");
									return NULL;
								}
								case SEM::Type::REFERENCE: {
									if(genLValue) {
										return builder_.CreatePointerCast(codeValue,
											genType(destType)->getPointerTo());
									} else {
										return builder_.CreatePointerCast(codeValue, genType(destType));
									}
								}
								case SEM::Type::POINTER: {
									if(genLValue) {
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
							
							if(sourceTarget->isInterface()) {
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
								// Get the vtable pointer.
								llvm::Value* vtablePointer = vtables_.get(sourceTarget->getObjectType());
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
							
							for(size_t i = 0; i < parameters.size(); i++) {
								SEM::Value* paramValue = parameters.at(i);
								genStore(genValue(paramValue),
									builder_.CreateConstInBoundsGEP2_32(objectValue, 0, i),
									paramValue->type());
							}
							
							return objectValue;
						}
						case SEM::Value::MEMBERACCESS: {
							const size_t offset = memberVarIds_.get(value->memberAccess.memberVar);
							if(genLValue) {
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
							
							if(returnType->isClassOrTemplateVar()) {
								returnValue = genAlloca(returnType);
								assert(returnValue != NULL &&
									"Must have lvalue for holding class "
									"return value so it can be passed by reference");
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
							llvm::Function* function = functions_.get(value->functionRef.function);
							assert(function != NULL && "FunctionRef requires a valid function");
							return function;
						}
						case SEM::Value::METHODOBJECT: {
							llvm::Value* function = genValue(value->methodObject.method);
							assert(function != NULL && "MethodObject requires a valid function");
							llvm::Value* dataPointer = generateLValue(value->methodObject.methodOwner);
							assert(dataPointer != NULL && "MethodObject requires a valid data pointer");
							llvm::Value* methodValue = llvm::UndefValue::get(genType(value->type()));
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
							
							SEM::Type* returnType = value->type();
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
			codeGen_->genNamespaceTypePlaceholders(Name::Absolute(), nameSpace);
			codeGen_->genNamespaceTypeMembers(Name::Absolute(), nameSpace);
			codeGen_->genNamespaceFunctionDecls(Name::Absolute(), nameSpace);
			codeGen_->genNamespaceVTables(Name::Absolute(), nameSpace);
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

