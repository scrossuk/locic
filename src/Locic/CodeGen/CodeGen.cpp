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


#include <assert.h>
#include <cstdio>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <Locic/SEM.h>
#include <Locic/CodeGen/CodeGen.h>

using namespace llvm;

class CodeGen{
	private:
		std::string name_;
		Module * module_;
		IRBuilder<> builder_;
		FunctionType * currentFunctionType_;
		Function * currentFunction_;
		BasicBlock * currentBasicBlock_;
		FunctionPassManager fpm_;
		std::map<SEM_TypeInstance *, StructType *> structs_;
		std::map<SEM_FunctionDecl *, Function *> functions_;
		std::map<std::size_t, AllocaInst *> localVariables_, paramVariables_;

	public:
		CodeGen(const char * moduleName)
			: name_(moduleName),
			module_(new Module(name_.c_str(), getGlobalContext())),
			builder_(getGlobalContext()),
			fpm_(module_){
			
			InitializeNativeTarget();
			
			TargetRegistry::iterator it;
			
			for(it = TargetRegistry::begin(); it != TargetRegistry::end(); ++it){
				std::cout << "Got target: name=" << (*it).getName() << ", description=" << (*it).getShortDescription() << std::endl;
			}
			
			std::cout << "Default target triple: " << sys::getHostTriple() << std::endl;
			
			
			
			std::string error;
			const Target * target = TargetRegistry::lookupTarget(sys::getHostTriple(), error);
			
			if(target != NULL){
				std::cout << "Default target: name=" << target->getName() << ", description=" << target->getShortDescription() << std::endl;
								
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
			}else{
				std::cout << "Error when looking up default target: " << error << std::endl;
			}
			
			if(target->hasTargetMachine()){
				std::auto_ptr<TargetMachine> targetMachine(target->createTargetMachine(sys::getHostTriple(), "", ""));
				const TargetData * targetData = targetMachine->getTargetData();
				if(targetData != 0){
					std::cout << "--Pointer size = " << targetData->getPointerSize() << std::endl;
					std::cout << "--Pointer size (in bits) = " << targetData->getPointerSizeInBits() << std::endl;
					std::cout << "--Little endian = " << (targetData->isLittleEndian() ? "true" : "false") << std::endl;
					std::cout << "--Big endian = " << (targetData->isBigEndian() ? "true" : "false") << std::endl;
					std::cout << "--Legal integer sizes = {";
					
					bool b = false;
					for(unsigned int i = 0; i < 1000; i++){
						if(targetData->isLegalInteger(i)){
							if(b) std::cout << ", ";
							std::cout << i;
							b = true;
						}
					}
					
					std::cout << "}" << std::endl;
				}
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
		
		~CodeGen(){
			delete module_;
		}
		
		void dump(){
			module_->dump();
		}
		
		void genFile(SEM_Module * module){
			assert(module != NULL);
			
			Locic_StringMap typeInstances = module->typeInstances;
			for(Locic_StringMapIterator typeIt = Locic_StringMap_Begin(typeInstances);
				Locic_StringMap_IsEnd(typeInstances, typeIt) == 0; Locic_StringMap_Advance(typeIt)){
				
				SEM_TypeInstance * typeInstance = reinterpret_cast<SEM_TypeInstance *>(Locic_StringMap_GetData(typeIt));
				assert(typeInstance != NULL);
				
				switch(typeInstance->typeEnum){
					case SEM_TYPEINST_STRUCT:
					{
						std::vector<Type *> structMembers;
						structMembers.push_back(Type::getInt1Ty(getGlobalContext()));
						structs_[typeInstance] = StructType::create(structMembers, typeInstance->name);
						break;
					}
					default:
					{
						std::cerr << "Unimplemented type with name '" << typeInstance->name << "'." << std::endl;
					}
				}
			}
			
			Locic_StringMap functionDecls = module->functionDeclarations;
			for(Locic_StringMapIterator declIt = Locic_StringMap_Begin(functionDecls);
				Locic_StringMap_IsEnd(functionDecls, declIt) == 0; Locic_StringMap_Advance(declIt)){
				
				SEM_FunctionDecl * decl = reinterpret_cast<SEM_FunctionDecl *>(Locic_StringMap_GetData(declIt));
				assert(decl != NULL);
				functions_[decl] = Function::Create(genFunctionType(decl->type), Function::ExternalLinkage, decl->name, module_);
			}
		
			Locic_List * functionDefs = module->functionDefinitions;
			for(Locic_ListElement * defIt = Locic_List_Begin(functionDefs); defIt != Locic_List_End(functionDefs); defIt = defIt->next){
				genFunctionDef(reinterpret_cast<SEM_FunctionDef *>(defIt->data));
			}
		}
		
		FunctionType * genFunctionType(SEM_Type * type){
			assert(type != NULL);
			assert(type->typeEnum == SEM_TYPE_FUNC);
			
			Type * returnType = genType(type->funcType.returnType);
			
			std::vector<Type *> paramTypes;
			Locic_ListElement * it;
			Locic_List * params = type->funcType.parameterTypes;
			for(it = Locic_List_Begin(params); it != Locic_List_End(params); it = it->next){
				paramTypes.push_back(genType(reinterpret_cast<SEM_Type *>(it->data)));
			}
			
			bool isVarArg = false;
			return FunctionType::get(returnType, paramTypes, isVarArg);
		}

		Type * genType(SEM_Type * type){
			switch(type->typeEnum){
				case SEM_TYPE_VOID:
				{
					return Type::getVoidTy(getGlobalContext());
				}
				case SEM_TYPE_BASIC:
				{
					switch(type->basicType.typeEnum){
						case SEM_TYPE_BASIC_INT:
							return Type::getInt32Ty(getGlobalContext());
						case SEM_TYPE_BASIC_BOOL:
							return Type::getInt1Ty(getGlobalContext());
						case SEM_TYPE_BASIC_FLOAT:
							return Type::getFloatTy(getGlobalContext());
						default:
							std::cerr << "CodeGen error: Unknown basic type." << std::endl;
							return Type::getVoidTy(getGlobalContext());
							
					}
				}
				case SEM_TYPE_NAMED:
				{
					SEM_TypeInstance * typeInstance = type->namedType.typeInstance;
					if(typeInstance->typeEnum == SEM_TYPEINST_STRUCT){
						StructType * structType = structs_[typeInstance];
						assert(structType != NULL);
						return structType;
					}else{
						std::cerr << "CodeGen error: Named type not implemented." << std::endl;
						return Type::getInt32Ty(getGlobalContext());
					}
				}
				case SEM_TYPE_PTR:
				{
					Type * ptrType = genType(type->ptrType.ptrType);
					if(ptrType->isVoidTy()){
						// LLVM doesn't support 'void *' => use 'int8_t *' instead.
						return PointerType::getUnqual(Type::getInt8Ty(getGlobalContext()));
					}else{
						return ptrType->getPointerTo();
					}
				}
				case SEM_TYPE_FUNC:
				{
					return genFunctionType(type)->getPointerTo();
				}
				default:
				{
					std::cerr << "CodeGen error: Unknown type." << std::endl;
					return Type::getVoidTy(getGlobalContext());
				}
			}
		}
		
		void genFunctionDef(SEM_FunctionDef * functionDef){
			// Create function.
			currentFunction_ = functions_[functionDef->declaration];
			assert(currentFunction_ != NULL);
			
			currentBasicBlock_ = BasicBlock::Create(getGlobalContext(), "entry", currentFunction_);
			builder_.SetInsertPoint(currentBasicBlock_);
			
			// Store arguments onto stack.
			Function::arg_iterator arg;
			
			Locic_List * param = functionDef->declaration->parameterVars;
			Locic_ListElement * it;
			for (it = Locic_List_Begin(param), arg = currentFunction_->arg_begin(); it != Locic_List_End(param); ++arg, it = it->next){
				SEM_Var * paramVar = reinterpret_cast<SEM_Var *>(it->data);
				
				// Create an alloca for this variable.
				AllocaInst * stackObject = builder_.CreateAlloca(genType(paramVar->type));
    				
    				paramVariables_[paramVar->varId] = stackObject;

    				// Store the initial value into the alloca.
    				builder_.CreateStore(arg, stackObject);
			}
			
			genScope(functionDef->scope);
			
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
		
		void genScope(SEM_Scope * scope){
			Locic_Array array = scope->localVariables;
			
			for(std::size_t i = 0; i < Locic_Array_Size(array); i++){
				SEM_Var * localVar = reinterpret_cast<SEM_Var *>(Locic_Array_Get(array, i));
				
				// Create an alloca for this variable.
    				AllocaInst * stackObject = builder_.CreateAlloca(genType(localVar->type));
    				
    				localVariables_[localVar->varId] = stackObject;
			}
		
			Locic_List * list = scope->statementList;
			
			for(Locic_ListElement * it = Locic_List_Begin(list); it != Locic_List_End(list); it = it->next){
				SEM_Statement * statement = reinterpret_cast<SEM_Statement *>(it->data);
				genStatement(statement);
			}
		}
		
		void genStatement(SEM_Statement * statement){
			switch(statement->type){
				case SEM_STATEMENT_VALUE:
				{
					genValue(statement->valueStmt.value);
					break;
				}
				case SEM_STATEMENT_SCOPE:
				{
					genScope(statement->scopeStmt.scope);
					break;
				}
				case SEM_STATEMENT_IF:
				{
					BasicBlock * thenBB = BasicBlock::Create(getGlobalContext(), "then", currentFunction_);
					BasicBlock * elseBB = BasicBlock::Create(getGlobalContext(), "else");
					BasicBlock * mergeBB = BasicBlock::Create(getGlobalContext(), "ifmerge");
					
					builder_.CreateCondBr(genValue(statement->ifStmt.cond), thenBB, elseBB);
					
					// Create 'then'.
					builder_.SetInsertPoint(thenBB);
					
					genScope(statement->ifStmt.ifTrue);
					
					builder_.CreateBr(mergeBB);
					
					// Create 'else'.
					currentFunction_->getBasicBlockList().push_back(elseBB);
					builder_.SetInsertPoint(elseBB);
					
					if(statement->ifStmt.ifFalse != NULL){
						genScope(statement->ifStmt.ifFalse);
					}
					
					builder_.CreateBr(mergeBB);
					
					// Create merge.
					currentFunction_->getBasicBlockList().push_back(mergeBB);
					builder_.SetInsertPoint(mergeBB);
					break;
				}
				case SEM_STATEMENT_WHILE:
				{
					BasicBlock * insideLoopBB = BasicBlock::Create(getGlobalContext(), "insideLoop", currentFunction_);
					BasicBlock * afterLoopBB = BasicBlock::Create(getGlobalContext(), "afterLoop");
					
					builder_.CreateCondBr(genValue(statement->whileStmt.cond), insideLoopBB, afterLoopBB);
					
					// Create loop contents.
					builder_.SetInsertPoint(insideLoopBB);
					
					genScope(statement->whileStmt.whileTrue);
					
					builder_.CreateCondBr(genValue(statement->whileStmt.cond), insideLoopBB, afterLoopBB);
					
					// Create 'else'.
					currentFunction_->getBasicBlockList().push_back(afterLoopBB);
					builder_.SetInsertPoint(afterLoopBB);
					break;
				}
				case SEM_STATEMENT_ASSIGN:
				{
					SEM_Value * lValue = statement->assignStmt.lValue;
					SEM_Value * rValue = statement->assignStmt.rValue;
					
					builder_.CreateStore(genValue(rValue), genValue(lValue, true));
					break;
				}
				case SEM_STATEMENT_RETURN:
				{
					if(statement->returnStmt.value != NULL){
						builder_.CreateRet(genValue(statement->returnStmt.value));
					}else{
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
		
		Value * genValue(SEM_Value * value, bool genLValue = false){
			switch(value->valueType){
				case SEM_VALUE_CONSTANT:
				{
					switch(value->constant.type){
						case SEM_CONSTANT_BOOL:
							return ConstantInt::get(getGlobalContext(), APInt(1, value->constant.boolConstant));
						case SEM_CONSTANT_INT:
							return ConstantInt::get(getGlobalContext(), APInt(32, value->constant.intConstant));
						case SEM_CONSTANT_FLOAT:
							return ConstantFP::get(getGlobalContext(), APFloat(value->constant.floatConstant));
						default:
							std::cerr << "CodeGen error: Unknown constant." << std::endl;
							return ConstantInt::get(getGlobalContext(), APInt(32, 0));
					}
				}
				case SEM_VALUE_COPY:
				{
					return genValue(value->copyValue.value);
				}
				case SEM_VALUE_VAR:
				{
					SEM_Var * var = value->varValue.var;
					switch(var->varType){
						case SEM_VAR_PARAM:
						{
							if(genLValue){
								return paramVariables_[var->varId];
							}else{
								return builder_.CreateLoad(paramVariables_[var->varId]);
							}
						}
						case SEM_VAR_LOCAL:
						{
							if(genLValue){
								return localVariables_[var->varId];
							}else{
								return builder_.CreateLoad(localVariables_[var->varId]);
							}
						}
						case SEM_VAR_THIS:
						{
							std::cerr << "CodeGen error: Unimplemented member variable access." << std::endl;
							return ConstantInt::get(getGlobalContext(), APInt(32, 1));
						}
						default:
						{
							std::cerr << "CodeGen error: Unknown variable type in variable access." << std::endl;
							return ConstantInt::get(getGlobalContext(), APInt(32, 0));
						}
					}
				}
				case SEM_VALUE_UNARY:
				{
					SEM_OpType opType = value->unary.opType;
					switch(value->unary.type){
						case SEM_UNARY_PLUS:
							assert(opType == SEM_OP_INT || opType == SEM_OP_FLOAT);
							return genValue(value->unary.value);
						case SEM_UNARY_MINUS:
							assert(opType == SEM_OP_INT || opType == SEM_OP_FLOAT);
							if(opType == SEM_OP_INT){
								return builder_.CreateNeg(genValue(value->unary.value));
							}else if(opType == SEM_OP_FLOAT){
								return builder_.CreateFNeg(genValue(value->unary.value));
							}
						case SEM_UNARY_NOT:
							assert(opType == SEM_OP_BOOL);
							return builder_.CreateNot(genValue(value->unary.value));
						case SEM_UNARY_ADDRESSOF:
							assert(opType == SEM_OP_PTR);
							return genValue(value->unary.value, true);
						case SEM_UNARY_DEREF:
							assert(opType == SEM_OP_PTR);
							if(genLValue){
								return genValue(value->unary.value);
							}else{
								return builder_.CreateLoad(genValue(value->unary.value));
							}
						default:
							std::cerr << "CodeGen error: Unknown unary bool operand." << std::endl;
							return genValue(value->unary.value);
					}
				}
				case SEM_VALUE_BINARY:
				{
					SEM_OpType opType = value->binary.opType;
					switch(value->binary.type){
						case SEM_BINARY_ADD:
							assert(opType == SEM_OP_INT || opType == SEM_OP_FLOAT);
							if(opType == SEM_OP_INT){
								return builder_.CreateAdd(genValue(value->binary.left), genValue(value->binary.right));
							}else{
								return builder_.CreateFAdd(genValue(value->binary.left), genValue(value->binary.right));
							}
							return builder_.CreateAdd(genValue(value->binary.left), genValue(value->binary.right));
						case SEM_BINARY_SUBTRACT:
							assert(opType == SEM_OP_INT || opType == SEM_OP_FLOAT);
							if(opType == SEM_OP_INT){
								return builder_.CreateSub(genValue(value->binary.left), genValue(value->binary.right));
							}else{
								return builder_.CreateFSub(genValue(value->binary.left), genValue(value->binary.right));
							}
							return builder_.CreateSub(genValue(value->binary.left), genValue(value->binary.right));
						case SEM_BINARY_MULTIPLY:
							assert(opType == SEM_OP_INT || opType == SEM_OP_FLOAT);
							if(opType == SEM_OP_INT){
								return builder_.CreateMul(genValue(value->binary.left), genValue(value->binary.right));
							}else{
								return builder_.CreateFMul(genValue(value->binary.left), genValue(value->binary.right));
							}
						case SEM_BINARY_DIVIDE:
							assert(opType == SEM_OP_INT || opType == SEM_OP_FLOAT);
							if(opType == SEM_OP_INT){
								return builder_.CreateSDiv(genValue(value->binary.left), genValue(value->binary.right));
							}else{
								return builder_.CreateFDiv(genValue(value->binary.left), genValue(value->binary.right));
							}
						case SEM_BINARY_ISEQUAL:
							assert(opType == SEM_OP_BOOL || opType == SEM_OP_INT || opType == SEM_OP_FLOAT);
							if(opType == SEM_OP_BOOL || opType == SEM_OP_INT){
								return builder_.CreateICmpEQ(genValue(value->binary.left), genValue(value->binary.right));
							}else{
								return builder_.CreateFCmpOEQ(genValue(value->binary.left), genValue(value->binary.right));
							}
						case SEM_BINARY_NOTEQUAL:
							assert(opType == SEM_OP_BOOL || opType == SEM_OP_INT || opType == SEM_OP_FLOAT);
							if(opType == SEM_OP_BOOL || opType == SEM_OP_INT){
								return builder_.CreateICmpNE(genValue(value->binary.left), genValue(value->binary.right));
							}else{
								return builder_.CreateFCmpONE(genValue(value->binary.left), genValue(value->binary.right));
							}
						case SEM_BINARY_LESSTHAN:
							assert(opType == SEM_OP_INT || opType == SEM_OP_FLOAT);
							if(opType == SEM_OP_INT){
								return builder_.CreateICmpSLT(genValue(value->binary.left), genValue(value->binary.right));
							}else{
								return builder_.CreateFCmpOLT(genValue(value->binary.left), genValue(value->binary.right));
							}
						case SEM_BINARY_GREATERTHAN:
							assert(opType == SEM_OP_INT || opType == SEM_OP_FLOAT);
							if(opType == SEM_OP_INT){
								return builder_.CreateICmpSGT(genValue(value->binary.left), genValue(value->binary.right));
							}else{
								return builder_.CreateFCmpOGT(genValue(value->binary.left), genValue(value->binary.right));
							}
						case SEM_BINARY_GREATEROREQUAL:
							assert(opType == SEM_OP_INT || opType == SEM_OP_FLOAT);
							if(opType == SEM_OP_INT){
								return builder_.CreateICmpSGE(genValue(value->binary.left), genValue(value->binary.right));
							}else{
								return builder_.CreateFCmpOGE(genValue(value->binary.left), genValue(value->binary.right));
							}
						case SEM_BINARY_LESSOREQUAL:
							assert(opType == SEM_OP_INT || opType == SEM_OP_FLOAT);
							if(opType == SEM_OP_INT){
								return builder_.CreateICmpSLE(genValue(value->binary.left), genValue(value->binary.right));
							}else{
								return builder_.CreateFCmpOLE(genValue(value->binary.left), genValue(value->binary.right));
							}
						default:
							std::cerr << "CodeGen error: Unknown binary operand." << std::endl;
							return ConstantInt::get(getGlobalContext(), APInt(32, 0));
					}
				}
				case SEM_VALUE_TERNARY:
				{
					return builder_.CreateSelect(genValue(value->ternary.condition), genValue(value->ternary.ifTrue, genLValue), genValue(value->ternary.ifFalse, genLValue));
				}
				case SEM_VALUE_CAST:
				{
					Value * codeValue = genValue(value->cast.value, genLValue);
					SEM_Type * sourceType = value->cast.value->type;
					SEM_Type * destType = value->type;
					
					assert(sourceType->typeEnum == destType->typeEnum);
					
					switch(sourceType->typeEnum){
						case SEM_TYPE_VOID:
						{
							return codeValue;
						}
						case SEM_TYPE_BASIC:
						{
							if(sourceType->basicType.typeEnum == destType->basicType.typeEnum){
								return codeValue;
							}
			
							// Int -> Float.
							if(sourceType->basicType.typeEnum == SEM_TYPE_BASIC_INT && destType->basicType.typeEnum == SEM_TYPE_BASIC_FLOAT){
								return builder_.CreateSIToFP(codeValue, genType(destType));
							}
			
							// Float -> Int.
							if(sourceType->basicType.typeEnum == SEM_TYPE_BASIC_FLOAT && destType->basicType.typeEnum == SEM_TYPE_BASIC_INT){
								return builder_.CreateFPToSI(codeValue, genType(destType));
							}
							
							return codeValue;
						}
						case SEM_TYPE_NAMED:
						case SEM_TYPE_PTR:
						{
							if(genLValue){
								return builder_.CreatePointerCast(codeValue, PointerType::getUnqual(genType(destType)));
							}else{
								return builder_.CreatePointerCast(codeValue, genType(destType));
							}
						}
						case SEM_TYPE_FUNC:
						{
							return codeValue;
						}
						default:
							std::cerr << "CodeGen error: Unknown type in cast." << std::endl;
							return 0;
					}
				}	
				case SEM_VALUE_CONSTRUCT:
					std::cerr << "CodeGen error: Unimplemented constructor call." << std::endl;
					return ConstantInt::get(getGlobalContext(), APInt(32, 42));
				case SEM_VALUE_MEMBERACCESS:
					std::cerr << "CodeGen error: Unimplemented member access." << std::endl;
					return ConstantInt::get(getGlobalContext(), APInt(32, 42));
				case SEM_VALUE_FUNCTIONCALL:
				{
					std::vector<Value *> parameters;
					
					Locic_List * list = value->functionCall.parameters;
					for(Locic_ListElement * it = Locic_List_Begin(list); it != Locic_List_End(list); it = it->next){
						SEM_Value * paramValue = reinterpret_cast<SEM_Value *>(it->data);
						parameters.push_back(genValue(paramValue));
					}
					
					return builder_.CreateCall(genValue(value->functionCall.functionValue), parameters);
				}
				case SEM_VALUE_FUNCTIONREF:
				{
					Function * function = functions_[value->functionRef.functionDecl];
					assert(function != NULL);
					return function;
				}
				default:
					std::cerr << "CodeGen error: Unknown value." << std::endl;
					return ConstantInt::get(getGlobalContext(), APInt(32, 0));
			}
		}
	
};

extern "C"{
	
	void * Locic_CodeGenAlloc(const char * moduleName){
		return new CodeGen(moduleName);
	}
	
	void Locic_CodeGenFree(void * context){
		delete reinterpret_cast<CodeGen *>(context);
	}
	
	void Locic_CodeGen(void * context, SEM_Module * module){
		reinterpret_cast<CodeGen *>(context)->genFile(module);
	}
	
	void Locic_CodeGenDump(void * context){
		reinterpret_cast<CodeGen *>(context)->dump();
	}
	
}

