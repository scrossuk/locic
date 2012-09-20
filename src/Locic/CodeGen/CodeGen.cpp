#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Basic/TargetInfo.h>

#include <assert.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <Locic/Map.hpp>
#include <Locic/SEM.hpp>
#include <Locic/CodeGen/CodeGen.hpp>

using namespace llvm;

class CodeGen {
	private:
		std::string name_;
		Module* module_;
		IRBuilder<> builder_;
		FunctionType* currentFunctionType_;
		Function* currentFunction_;
		BasicBlock* currentBasicBlock_;
		FunctionPassManager fpm_;
		Locic::Map<SEM::TypeInstance*, Type*> typeInstances_;
		Locic::Map<SEM::Function*, Function*> functions_;
		std::vector<AllocaInst*> localVariables_, paramVariables_;
		clang::TargetInfo* targetInfo_;
		Value * returnVar_;
		
	public:
		CodeGen(const std::string& moduleName)
			: name_(moduleName),
			  module_(new Module(name_.c_str(), getGlobalContext())),
			  builder_(getGlobalContext()),
			  fpm_(module_),
			  targetInfo_(0),
			  returnVar_(NULL){
			  
			InitializeNativeTarget();
			
			std::cout << "Default target triple: " << sys::getDefaultTargetTriple() << std::endl;
			
			module_->setTargetTriple(sys::getDefaultTargetTriple());
			
			std::string error;
			const Target* target = TargetRegistry::lookupTarget(sys::getDefaultTargetTriple(), error);
			
			if(target != NULL) {
				std::cout << "Target: name=" << target->getName() << ", description=" << target->getShortDescription() << std::endl;
				
				std::cout << "--Does " << (target->hasJIT() ? "" : "not ") << "support just-in-time compilation." << std::endl;
				std::cout << "--Does " << (target->hasTargetMachine() ? "" : "not ") << "support code generation." << std::endl;
				std::cout << "--Does " << (target->hasMCAsmBackend() ? "" : "not ") << "support .o generation." << std::endl;
				std::cout << "--Does " << (target->hasMCAsmLexer() ? "" : "not ") << "support .s lexing." << std::endl;
				std::cout << "--Does " << (target->hasMCAsmParser() ? "" : "not ") << "support .s parsing." << std::endl;
				std::cout << "--Does " << (target->hasAsmPrinter() ? "" : "not ") << "support .s printing." << std::endl;
				std::cout << "--Does " << (target->hasMCDisassembler() ? "" : "not ") << "support disassembling." << std::endl;
				std::cout << "--Does " << (target->hasMCInstPrinter() ? "" : "not ") << "support printing instructions." << std::endl;
				std::cout << "--Does " << (target->hasMCCodeEmitter() ? "" : "not ") << "support instruction encoding." << std::endl;
				std::cout << "--Does " << (target->hasMCObjectStreamer() ? "" : "not ") << "support streaming to files." << std::endl;
				std::cout << "--Does " << (target->hasAsmStreamer() ? "" : "not ") << "support streaming ASM to files." << std::endl;
				
				if(target->hasTargetMachine()) {
					std::auto_ptr<TargetMachine> targetMachine(target->createTargetMachine(sys::getDefaultTargetTriple(), "", "", TargetOptions()));
					const TargetData* targetData = targetMachine->getTargetData();
					
					if(targetData != 0) {
						std::cout << "--Pointer size = " << targetData->getPointerSize() << std::endl;
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
						std::cout << std::endl;
						
						clang::CompilerInstance ci;
						ci.createDiagnostics(0, NULL);
						
						clang::TargetOptions to;
						to.Triple = sys::getDefaultTargetTriple();
						targetInfo_ = clang::TargetInfo::CreateTargetInfo(ci.getDiagnostics(), to);
						
						std::cout << "Information from Clang:" << std::endl;
						std::cout << "--Short width: " << targetInfo_->getShortWidth() << ", " << (sizeof(short) * 8) << std::endl;
						std::cout << "--Int width: " << targetInfo_->getIntWidth() << ", " << (sizeof(int) * 8) << std::endl;
						std::cout << "--Long width: " << targetInfo_->getLongWidth() << ", " << (sizeof(long) * 8) << std::endl;
						std::cout << "--Long long width: " << targetInfo_->getLongLongWidth() << ", " << (sizeof(long long) * 8) << std::endl;
						std::cout << "--Float width: " << targetInfo_->getFloatWidth() << ", " << (sizeof(float) * 8) << std::endl;
						std::cout << "--Double width: " << targetInfo_->getDoubleWidth() << ", " << (sizeof(double) * 8) << std::endl;
						std::cout << std::endl;
					}
				}
			} else {
				std::cout << "Error when looking up default target: " << error << std::endl;
			}
			
			// Set up the optimizer pipeline.
			// Provide basic AliasAnalysis support for GVN.
			fpm_.add(createBasicAliasAnalysisPass());
			// Promote allocas to registers.
			fpm_.add(createPromoteMemoryToRegisterPass());
			// Do simple "peephole" optimizations and bit-twiddling optzns.
			fpm_.add(createInstructionCombiningPass());
			// Reassociate expressions.
			fpm_.add(createReassociatePass());
			// Eliminate Common SubExpressions.
			fpm_.add(createGVNPass());
			// Simplify the control flow graph (deleting unreachable blocks, etc).
			fpm_.add(createCFGSimplificationPass());
			
			fpm_.doInitialization();
		}
		
		~CodeGen() {
			delete module_;
		}
		
		void dump() {
			module_->dump();
		}
		
		void genFile(SEM::Module* module) {
			assert(module != NULL);
			
			for(std::size_t i = 0; i < module->functions.size(); i++){
				genFunctionDef(module->functions.at(i));
			}
			
			for(std::size_t i = 0; i < module->typeInstances.size(); i++){
				SEM::TypeInstance * typeInstance = module->typeInstances.at(i);
				
				// TODO: generate class records.
				(void) typeInstance;
			}
		}
		
		// Lazy generation - function declarations are only
		// generated when they are first used by code.
		Function * genFunctionDecl(SEM::Function * function){
			assert(function != NULL);
			
			Locic::Optional<Function *> optionalFunction = functions_.tryGet(function);
			if(optionalFunction.hasValue()) return optionalFunction.getValue();
			
			Function * functionDecl = Function::Create(genFunctionType(function->type), Function::ExternalLinkage, function->name.genString(), module_);
			
			functions_.insert(function, functionDecl);
			return functionDecl;
		}
		
		// Lazy generation - struct types are only
		// generated when they are first used by code.
		Type* genStructType(SEM::TypeInstance * typeInstance){
			assert(typeInstance != NULL);
			
			Locic::Optional<Type *> optionalStruct = typeInstances_.tryGet(typeInstance);
			if(optionalStruct.hasValue()) return optionalStruct.getValue();
			
			StructType * structType = StructType::create(getGlobalContext(), typeInstance->name.genString());
			
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
			
				std::vector<Type*> memberVariables(paramOffset + typeInstance->variables.size(), NULL);
				
				if(typeInstance->typeEnum == SEM::TypeInstance::CLASSDEF){
					// Add class record pointer.
					memberVariables.front() = PointerType::getUnqual(Type::getInt8Ty(getGlobalContext()));
				}
				
				Locic::StringMap<SEM::Var *>::Range range = typeInstance->variables.range();
				for(; !range.empty(); range.popFront()){
					SEM::Var * var = range.front().value();
					assert(memberVariables.at(paramOffset + var->id) == NULL);
					memberVariables.at(paramOffset + var->id) = genType(var->type);
				}
				
				structType->setBody(memberVariables);
			}else{
				// Generating the type for a class declaration, so the size is
				// currently unknown (and will be known at load-time/run-time).
				std::vector<Type *> memberVariables;
				
				// Pointer to class record.
				memberVariables.push_back(PointerType::getUnqual(Type::getInt8Ty(getGlobalContext())));
				
				// Zero length array indicates this structure is variable length,
				// or in other words, currently unknown for this code.
				memberVariables.push_back(ArrayType::get(Type::getInt8Ty(getGlobalContext()), 0));
				structType->setBody(memberVariables);
			}
			
			return structType;
		}
		
		FunctionType* genFunctionType(SEM::Type* type) {
			assert(type != NULL);
			assert(type->typeEnum == SEM::Type::FUNCTION);
			
			SEM::Type * semReturnType = type->functionType.returnType;
			Type* returnType = genType(semReturnType);
			
			std::vector<Type*> paramTypes;
			
			if(semReturnType->typeEnum == SEM::Type::NAMED){
				paramTypes.push_back(returnType->getPointerTo());
				returnType = Type::getVoidTy(getGlobalContext());
			}
			
			const std::vector<SEM::Type *>& params = type->functionType.parameterTypes;
			
			for(std::size_t i = 0; i < params.size(); i++){
				paramTypes.push_back(genType(params.at(i)));
			}
			
			const bool isVarArg = false;
			return FunctionType::get(returnType, paramTypes, isVarArg);
		}
		
		Type* genType(SEM::Type* type) {
			switch(type->typeEnum) {
				case SEM::Type::VOID: {
					return Type::getVoidTy(getGlobalContext());
				}
				case SEM::Type::NULLT: {
					return PointerType::getUnqual(Type::getInt8Ty(getGlobalContext()));
				}
				case SEM::Type::BASIC: {
					switch(type->basicType.typeEnum) {
						case SEM::Type::BasicType::INTEGER:
							return IntegerType::get(getGlobalContext(), targetInfo_->getLongWidth());
						case SEM::Type::BasicType::BOOLEAN:
							return Type::getInt1Ty(getGlobalContext());
						case SEM::Type::BasicType::FLOAT:
							return Type::getFloatTy(getGlobalContext());
						default:
							std::cerr << "CodeGen error: Unknown basic type." << std::endl;
							return Type::getVoidTy(getGlobalContext());
					}
				}
				case SEM::Type::NAMED: {
					return genStructType(type->namedType.typeInstance);
				}
				case SEM::Type::POINTER: {
					Type* pointerType = genType(type->pointerType.targetType);
					
					if(pointerType->isVoidTy()) {
						// LLVM doesn't support 'void *' => use 'int8_t *' instead.
						return PointerType::getUnqual(Type::getInt8Ty(getGlobalContext()));
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
					
					std::vector<Type*> types;
					types.push_back(genFunctionType(type->methodType.functionType)->getPointerTo());
					types.push_back(genType(pointerToObjectType));
					return StructType::get(getGlobalContext(), types);
				}
				default: {
					std::cerr << "CodeGen error: Unknown type." << std::endl;
					return Type::getVoidTy(getGlobalContext());
				}
			}
		}
		
		void genFunctionDef(SEM::Function* function) {
			assert(function != NULL);
			
			if(function->scope == NULL) return;
		
			currentFunction_ = genFunctionDecl(function);
			assert(currentFunction_ != NULL);
			
			currentBasicBlock_ = BasicBlock::Create(getGlobalContext(), "entry", currentFunction_);
			builder_.SetInsertPoint(currentBasicBlock_);
			
			// Store arguments onto stack.
			Function::arg_iterator arg = currentFunction_->arg_begin();
			
			SEM::Type * returnType = function->type->functionType.returnType;
			
			if(returnType->typeEnum == SEM::Type::NAMED){
				returnVar_ = arg++;
			}else{
				returnVar_ = NULL;
			}
			
			const std::vector<SEM::Var *>& parameterVars = function->parameters;
			
			for(std::size_t i = 0; i < parameterVars.size(); ++arg, i++){
				SEM::Var* paramVar = parameterVars.at(i);
				
				// Create an alloca for this variable.
				AllocaInst* stackObject = builder_.CreateAlloca(genType(paramVar->type));
				
				assert(paramVar->id == paramVariables_.size());
				paramVariables_.push_back(stackObject);
				
				// Store the initial value into the alloca.
				builder_.CreateStore(arg, stackObject);
			}
			
			genScope(function->scope);
			
			// Need to terminate the final basic block.
			// (just make it loop to itself - this will
			// be removed by dead code elimination)
			builder_.CreateBr(builder_.GetInsertBlock());
			
			std::cout << "---Before verification:" << std::endl;
			
			module_->dump();
			
			// Check the generated function is correct.
			verifyFunction(*currentFunction_);
			
			std::cout << "---Before optimisation:" << std::endl;
			
			module_->dump();
			
			std::cout << "---Running optimisations..." << std::endl;
			
			// Run optimisations.
			fpm_.run(*currentFunction_);
			
			paramVariables_.clear();
			localVariables_.clear();
		}
		
		void genScope(SEM::Scope* scope) {
			for(std::size_t i = 0; i < scope->localVariables.size(); i++) {
				SEM::Var* localVar = scope->localVariables.at(i);
				
				// Create an alloca for this variable.
				AllocaInst* stackObject = builder_.CreateAlloca(genType(localVar->type));
				
				assert(localVar->id == localVariables_.size());
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
					BasicBlock* thenBB = BasicBlock::Create(getGlobalContext(), "then", currentFunction_);
					BasicBlock* elseBB = BasicBlock::Create(getGlobalContext(), "else");
					BasicBlock* mergeBB = BasicBlock::Create(getGlobalContext(), "ifmerge");
					
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
					BasicBlock* insideLoopBB = BasicBlock::Create(getGlobalContext(), "insideLoop", currentFunction_);
					BasicBlock* afterLoopBB = BasicBlock::Create(getGlobalContext(), "afterLoop");
					
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
					if(statement->returnStmt.value != NULL) {
						Value * returnValue = genValue(statement->returnStmt.value);
						if(returnVar_ != NULL){
							returnValue->dump();
							returnVar_->dump();
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
					builder_.SetInsertPoint(BasicBlock::Create(getGlobalContext(), "next", currentFunction_));
					break;
				}
				default:
					std::cerr << "CodeGen error: Unknown statement." << std::endl;
			}
		}
		
		Value * generateLValue(SEM::Value * value){
			if(value->type->isLValue){
				return genValue(value, true);
			}else{
				Value * lValue = builder_.CreateAlloca(genType(value->type));
				Value * rValue = genValue(value);
				builder_.CreateStore(rValue, lValue);
				return lValue;
			}
		}
		
		Value* genValue(SEM::Value* value, bool genLValue = false) {
			switch(value->typeEnum) {
				case SEM::Value::CONSTANT: {
					switch(value->constant.typeEnum) {
						case SEM::Value::Constant::BOOLEAN:
							return ConstantInt::get(getGlobalContext(), APInt(1, value->constant.boolConstant));
						case SEM::Value::Constant::INTEGER:
							return ConstantInt::get(getGlobalContext(), APInt(targetInfo_->getLongWidth(), value->constant.intConstant));
						case SEM::Value::Constant::FLOAT:
							return ConstantFP::get(getGlobalContext(), APFloat(value->constant.floatConstant));
						case SEM::Value::Constant::NULLVAL:
							return ConstantPointerNull::get(PointerType::getUnqual(Type::getInt8Ty(getGlobalContext())));
						default:
							std::cerr << "CodeGen error: Unknown constant." << std::endl;
							return UndefValue::get(Type::getVoidTy(getGlobalContext()));
					}
				}
				case SEM::Value::COPY: {
					return genValue(value->copyValue.value);
				}
				case SEM::Value::VAR: {
					SEM::Var* var = value->varValue.var;
					
					switch(var->typeEnum) {
						case SEM::Var::PARAM: {
							Value * val = paramVariables_.at(var->id);
							assert(val != NULL);
							if(genLValue) {
								return val;
							} else {
								return builder_.CreateLoad(val);
							}
						}
						case SEM::Var::LOCAL: {
							Value * val = localVariables_.at(var->id);
							assert(val != NULL);
							if(genLValue) {
								return val;
							} else {
								return builder_.CreateLoad(val);
							}
						}
						case SEM::Var::MEMBER: {
							assert(!paramVariables_.empty());
							Value * object = paramVariables_.front();
							
							Value * memberPtr = builder_.CreateConstInBoundsGEP2_32(builder_.CreateLoad(object), 0, var->id + 1);
							
							if(genLValue){
								return memberPtr;
							}else{
								return builder_.CreateLoad(memberPtr);
							}
						}
						default: {
							std::cerr << "CodeGen error: Unknown variable type in variable access." << std::endl;
							return ConstantInt::get(getGlobalContext(), APInt(32, 0));
						}
					}
				}
				case SEM::Value::UNARY: {
					SEM::Value::Op::TypeEnum opType = value->unary.opType;
					
					switch(value->unary.typeEnum) {
						case SEM::Value::Unary::PLUS:
							assert(opType == SEM::Value::Op::INTEGER || opType == SEM::Value::Op::FLOAT);
							return genValue(value->unary.value);
						case SEM::Value::Unary::MINUS:
							assert(opType == SEM::Value::Op::INTEGER || opType == SEM::Value::Op::FLOAT);
							
							if(opType == SEM::Value::Op::INTEGER) {
								return builder_.CreateNeg(genValue(value->unary.value));
							} else if(opType == SEM::Value::Op::FLOAT) {
								return builder_.CreateFNeg(genValue(value->unary.value));
							}
							
						case SEM::Value::Unary::NOT:
							assert(opType == SEM::Value::Op::BOOLEAN);
							return builder_.CreateNot(genValue(value->unary.value));
						case SEM::Value::Unary::ADDRESSOF:
							assert(opType == SEM::Value::Op::POINTER);
							return genValue(value->unary.value, true);
						case SEM::Value::Unary::DEREF:
							assert(opType == SEM::Value::Op::POINTER);
							
							if(genLValue) {
								return genValue(value->unary.value);
							} else {
								return builder_.CreateLoad(genValue(value->unary.value));
							}
							
						default:
							std::cerr << "CodeGen error: Unknown unary bool operand." << std::endl;
							return genValue(value->unary.value);
					}
				}
				case SEM::Value::BINARY: {
					SEM::Value::Op::TypeEnum opType = value->binary.opType;
					
					switch(value->binary.typeEnum) {
						case SEM::Value::Binary::ADD:
							assert(opType == SEM::Value::Op::INTEGER || opType == SEM::Value::Op::FLOAT);
							
							if(opType == SEM::Value::Op::INTEGER) {
								return builder_.CreateAdd(genValue(value->binary.left), genValue(value->binary.right));
							} else {
								return builder_.CreateFAdd(genValue(value->binary.left), genValue(value->binary.right));
							}
							
							return builder_.CreateAdd(genValue(value->binary.left), genValue(value->binary.right));
						case SEM::Value::Binary::SUBTRACT:
							assert(opType == SEM::Value::Op::INTEGER || opType == SEM::Value::Op::FLOAT);
							
							if(opType == SEM::Value::Op::INTEGER) {
								return builder_.CreateSub(genValue(value->binary.left), genValue(value->binary.right));
							} else {
								return builder_.CreateFSub(genValue(value->binary.left), genValue(value->binary.right));
							}
							
							return builder_.CreateSub(genValue(value->binary.left), genValue(value->binary.right));
						case SEM::Value::Binary::MULTIPLY:
							assert(opType == SEM::Value::Op::INTEGER || opType == SEM::Value::Op::FLOAT);
							
							if(opType == SEM::Value::Op::INTEGER) {
								return builder_.CreateMul(genValue(value->binary.left), genValue(value->binary.right));
							} else {
								return builder_.CreateFMul(genValue(value->binary.left), genValue(value->binary.right));
							}
							
						case SEM::Value::Binary::DIVIDE:
							assert(opType == SEM::Value::Op::INTEGER || opType == SEM::Value::Op::FLOAT);
							
							if(opType == SEM::Value::Op::INTEGER) {
								return builder_.CreateSDiv(genValue(value->binary.left), genValue(value->binary.right));
							} else {
								return builder_.CreateFDiv(genValue(value->binary.left), genValue(value->binary.right));
							}
							
						case SEM::Value::Binary::ISEQUAL:
							if(opType == SEM::Value::Op::BOOLEAN || opType == SEM::Value::Op::INTEGER || opType == SEM::Value::Op::POINTER) {
								return builder_.CreateICmpEQ(genValue(value->binary.left), genValue(value->binary.right));
							} else {
								return builder_.CreateFCmpOEQ(genValue(value->binary.left), genValue(value->binary.right));
							}
							
						case SEM::Value::Binary::NOTEQUAL:
							if(opType == SEM::Value::Op::BOOLEAN || opType == SEM::Value::Op::INTEGER || opType == SEM::Value::Op::POINTER) {
								return builder_.CreateICmpNE(genValue(value->binary.left), genValue(value->binary.right));
							} else {
								return builder_.CreateFCmpONE(genValue(value->binary.left), genValue(value->binary.right));
							}
							
						case SEM::Value::Binary::LESSTHAN:
							assert(opType == SEM::Value::Op::INTEGER || opType == SEM::Value::Op::FLOAT || opType == SEM::Value::Op::POINTER);
							
							if(opType == SEM::Value::Op::INTEGER) {
								return builder_.CreateICmpSLT(genValue(value->binary.left), genValue(value->binary.right));
							} else if(opType == SEM::Value::Op::POINTER) {
								return builder_.CreateICmpULT(genValue(value->binary.left), genValue(value->binary.right));
							} else {
								return builder_.CreateFCmpOLT(genValue(value->binary.left), genValue(value->binary.right));
							}
							
						case SEM::Value::Binary::GREATERTHAN:
							assert(opType == SEM::Value::Op::INTEGER || opType == SEM::Value::Op::FLOAT || opType == SEM::Value::Op::POINTER);
							
							if(opType == SEM::Value::Op::INTEGER) {
								return builder_.CreateICmpSGT(genValue(value->binary.left), genValue(value->binary.right));
							} else if(opType == SEM::Value::Op::POINTER) {
								return builder_.CreateICmpUGT(genValue(value->binary.left), genValue(value->binary.right));
							} else {
								return builder_.CreateFCmpOGT(genValue(value->binary.left), genValue(value->binary.right));
							}
							
						case SEM::Value::Binary::GREATEROREQUAL:
							assert(opType == SEM::Value::Op::INTEGER || opType == SEM::Value::Op::FLOAT || opType == SEM::Value::Op::POINTER);
							
							if(opType == SEM::Value::Op::INTEGER) {
								return builder_.CreateICmpSGE(genValue(value->binary.left), genValue(value->binary.right));
							} else if(opType == SEM::Value::Op::POINTER) {
								return builder_.CreateICmpUGE(genValue(value->binary.left), genValue(value->binary.right));
							} else {
								return builder_.CreateFCmpOGE(genValue(value->binary.left), genValue(value->binary.right));
							}
							
						case SEM::Value::Binary::LESSOREQUAL:
							assert(opType == SEM::Value::Op::INTEGER || opType == SEM::Value::Op::FLOAT || opType == SEM::Value::Op::POINTER);
							
							if(opType == SEM::Value::Op::INTEGER) {
								return builder_.CreateICmpSLE(genValue(value->binary.left), genValue(value->binary.right));
							} else if(opType == SEM::Value::Op::POINTER) {
								return builder_.CreateICmpULE(genValue(value->binary.left), genValue(value->binary.right));
							} else {
								return builder_.CreateFCmpOLE(genValue(value->binary.left), genValue(value->binary.right));
							}
							
						default:
							std::cerr << "CodeGen error: Unknown binary operand." << std::endl;
							return ConstantInt::get(getGlobalContext(), APInt(32, 0));
					}
				}
				case SEM::Value::TERNARY: {
					return builder_.CreateSelect(genValue(value->ternary.condition), genValue(value->ternary.ifTrue, genLValue), genValue(value->ternary.ifFalse, genLValue));
				}
				case SEM::Value::CAST: {
					Value* codeValue = genValue(value->cast.value, genLValue);
					SEM::Type* sourceType = value->cast.value->type;
					SEM::Type* destType = value->type;
					
					assert(sourceType->typeEnum == destType->typeEnum || sourceType->typeEnum == SEM::Type::NULLT);
					
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
									assert(typeInstance->typeEnum != SEM::TypeInstance::STRUCT);
									
									Type * structType = genStructType(typeInstance);
									Value * structValue = UndefValue::get(structType);
									
									// Set class record pointer to NULL.
									Value * nullRecordPointer = ConstantPointerNull::get(PointerType::getUnqual(Type::getInt8Ty(getGlobalContext())));
									
									structType->dump();
									nullRecordPointer->dump();
									structValue = builder_.CreateInsertValue(structValue, nullRecordPointer, std::vector<unsigned>(1, 0));
									
									return structValue;
								}
								default:
								{
									std::cerr << "Internal compiler error: Invalid cast from null." << std::endl;
									return UndefValue::get(Type::getVoidTy(getGlobalContext()));
								}
							}
						}
						case SEM::Type::BASIC: {
							if(sourceType->basicType.typeEnum == destType->basicType.typeEnum) {
								return codeValue;
							}
							
							// Int -> Float.
							if(sourceType->basicType.typeEnum == SEM::Type::BasicType::INTEGER && destType->basicType.typeEnum == SEM::Type::BasicType::FLOAT) {
								return builder_.CreateSIToFP(codeValue, genType(destType));
							}
							
							// Float -> Int.
							if(sourceType->basicType.typeEnum == SEM::Type::BasicType::FLOAT && destType->basicType.typeEnum == SEM::Type::BasicType::INTEGER) {
								return builder_.CreateFPToSI(codeValue, genType(destType));
							}
							
							return codeValue;
						}
						case SEM::Type::NAMED:
						case SEM::Type::POINTER: {
							if(genLValue) {
								return builder_.CreatePointerCast(codeValue, PointerType::getUnqual(genType(destType)));
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
							std::cerr << "CodeGen error: Unknown type in cast." << std::endl;
							return UndefValue::get(Type::getVoidTy(getGlobalContext()));
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
					std::vector<Value*> parameters;
					
					const std::vector<SEM::Value *>& paramList = value->functionCall.parameters;
					
					SEM::Type * returnType = value->type;
					Value * returnValue = NULL;
					
					if(returnType->typeEnum == SEM::Type::NAMED){
						returnValue = builder_.CreateAlloca(genType(returnType));
						assert(returnValue != NULL);
						parameters.push_back(returnValue);
					}
					
					for(std::size_t i = 0; i < paramList.size(); i++){
						parameters.push_back(genValue(paramList.at(i)));
					}
					
					Value * callReturnValue = builder_.CreateCall(genValue(value->functionCall.functionValue), parameters);
					
					if(returnValue != NULL){
						return builder_.CreateLoad(returnValue);
					}else{
						return callReturnValue;
					}
				}
				case SEM::Value::FUNCTIONREF: {
					Function* function = genFunctionDecl(value->functionRef.function);
					assert(function != NULL);
					return function;
				}
				case SEM::Value::METHODOBJECT: {
					Function* function = genFunctionDecl(value->methodObject.method);
					assert(function != NULL);
					
					Value* dataPointer = generateLValue(value->methodObject.methodOwner);
					assert(dataPointer != NULL);
					
					Value * methodValue = UndefValue::get(genType(value->type));
					methodValue = builder_.CreateInsertValue(methodValue, function, std::vector<unsigned>(1, 0));
					methodValue = builder_.CreateInsertValue(methodValue, dataPointer, std::vector<unsigned>(1, 1));
					return methodValue;
				}
				case SEM::Value::METHODCALL: {
					Value * method = genValue(value->methodCall.methodValue);
					
					Value * function = builder_.CreateExtractValue(method, std::vector<unsigned>(1, 0));
					Value * dataPointer = builder_.CreateExtractValue(method, std::vector<unsigned>(1, 1));
					
					std::vector<Value*> parameters;
					parameters.push_back(dataPointer);
					
					const std::vector<SEM::Value *>& paramList = value->methodCall.parameters;
					for(std::size_t i = 0; i < paramList.size(); i++){
						parameters.push_back(genValue(paramList.at(i)));
					}
					
					return builder_.CreateCall(function, parameters);
				}
				default:
					std::cerr << "CodeGen error: Unknown value." << std::endl;
					return ConstantInt::get(getGlobalContext(), APInt(32, 0));
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

