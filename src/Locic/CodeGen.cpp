#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>

#include <Locic/CodeGen.h>
#include <Locic/SEM.h>

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
			Locic_List * functions = module->functionDefinitions;
			for(Locic_ListElement * it = Locic_List_Begin(functions); it != Locic_List_End(functions); it = it->next){
				genFunctionDef(reinterpret_cast<SEM_FunctionDef *>(it->data));
			}
		}

		Type * genType(SEM_Type * type){
			switch(type->typeEnum){
				case SEM_TYPE_BASIC:
				{
					switch(type->basicType.typeEnum){
						case SEM_TYPE_BASIC_VOID:
							return Type::getVoidTy(getGlobalContext());
						case SEM_TYPE_BASIC_INT:
							return Type::getInt32Ty(getGlobalContext());
						case SEM_TYPE_BASIC_BOOL:
							return Type::getInt32Ty(getGlobalContext());
						case SEM_TYPE_BASIC_FLOAT:
							return Type::getFloatTy(getGlobalContext());
						default:
							std::cout << "CodeGen error: Unknown basic type." << std::endl;
							return Type::getVoidTy(getGlobalContext());
							
					}
				}
				case SEM_TYPE_CLASS:
				{
					std::cout << "CodeGen error: Class type not implemented." << std::endl;
					return Type::getInt32Ty(getGlobalContext());
				}
				case SEM_TYPE_PTR:
				{
					return PointerType::get(genType(type->ptrType.ptrType), 0);
				}
				default:
				{
					std::cout << "CodeGen error: Unknown type." << std::endl;
					return Type::getVoidTy(getGlobalContext());
				}
			}
		}
		
		void genFunctionDef(SEM_FunctionDef * functionDef){
			Locic_List * functionParameters = functionDef->declaration->parameterVars;
			Locic_ListElement * it;
			
			std::vector<Type *> parameterTypes;
			
			// Get parameter types.
			for (it = Locic_List_Begin(functionParameters); it != Locic_List_End(functionParameters); it = it->next){
				SEM_Var * paramVar = reinterpret_cast<SEM_Var *>(it->data);
				parameterTypes.push_back(genType(paramVar->type));
			}
		
			currentFunctionType_ = FunctionType::get(genType(functionDef->declaration->returnType), parameterTypes, false);
	                
	                // Create function.
			currentFunction_ = Function::Create(currentFunctionType_, Function::ExternalLinkage, functionDef->declaration->name, module_);
			
			currentBasicBlock_ = BasicBlock::Create(getGlobalContext(), "entry", currentFunction_);
			builder_.SetInsertPoint(currentBasicBlock_);
			
			// Store arguments onto stack.
			Function::arg_iterator arg;
			
			for (it = Locic_List_Begin(functionParameters), arg = currentFunction_->arg_begin(); it != Locic_List_End(functionParameters); ++arg, it = it->next){
				SEM_Var * paramVar = reinterpret_cast<SEM_Var *>(it->data);
				
				// Create an alloca for this variable.
				AllocaInst * stackObject = builder_.CreateAlloca(genType(paramVar->type));
    				
    				paramVariables_[paramVar->varId] = stackObject;

    				// Store the initial value into the alloca.
    				builder_.CreateStore(arg, stackObject);
			}
			
			genScope(functionDef->scope);
			
			verifyFunction(*currentFunction_);
			
			//fpm_.run(*currentFunction_);
			
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
					genValue(statement->valueStmt.value);
					break;
				case SEM_STATEMENT_IF:
					std::cout << "CodeGen error: Unimplemented IF statement." << std::endl;
					break;
				case SEM_STATEMENT_ASSIGNVAR:
				{
					SEM_Var * var = statement->assignVar.var;
					switch(var->varType){
						case SEM_VAR_LOCAL:
						{
							builder_.CreateStore(genValue(statement->assignVar.value), localVariables_[var->varId]);
							break;
						}
						case SEM_VAR_THIS:
							break;
						default:
							std::cout << "CodeGen error: Unknown variable type in assignment statement." << std::endl;
					}
					break;
				}
				case SEM_STATEMENT_RETURN:
					builder_.CreateRet(genValue(statement->returnStmt.value));
					break;
				default:
					std::cout << "CodeGen error: Unknown statement." << std::endl;
			}
		}
		
		Value * genValue(SEM_Value * value, bool genLValue = false){
			switch(value->valueType){
				case SEM_VALUE_CONSTANT:
				{
					switch(value->constant.type){
						case SEM_CONSTANT_BOOL:
							return ConstantInt::get(getGlobalContext(), APInt(32, value->constant.boolConstant));
						case SEM_CONSTANT_INT:
							return ConstantInt::get(getGlobalContext(), APInt(32, value->constant.intConstant));
						case SEM_CONSTANT_FLOAT:
							return ConstantFP::get(getGlobalContext(), APFloat(value->constant.floatConstant));
						default:
							std::cout << "CodeGen error: Unknown constant." << std::endl;
							return ConstantInt::get(getGlobalContext(), APInt(32, 0));
					}
				}
				case SEM_VALUE_VARACCESS:
				{
					SEM_Var * var = value->varAccess.var;
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
							std::cout << "CodeGen error: Unimplemented member variable access." << std::endl;
							return ConstantInt::get(getGlobalContext(), APInt(32, 1));
						}
						default:
						{
							std::cout << "CodeGen error: Unknown variable type in variable access." << std::endl;
							return ConstantInt::get(getGlobalContext(), APInt(32, 0));
						}
					}
					return ConstantInt::get(getGlobalContext(), APInt(32, 1));
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
							std::cout << "CodeGen error: Unknown unary bool operand." << std::endl;
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
							return builder_.CreateICmpEQ(genValue(value->binary.left), genValue(value->binary.right));
						case SEM_BINARY_NOTEQUAL:
							return builder_.CreateICmpNE(genValue(value->binary.left), genValue(value->binary.right));
						case SEM_BINARY_GREATEROREQUAL:
							return builder_.CreateICmpSGE(genValue(value->binary.left), genValue(value->binary.right));
						case SEM_BINARY_LESSOREQUAL:
							return builder_.CreateICmpSLE(genValue(value->binary.left), genValue(value->binary.right));
						default:
							std::cout << "CodeGen error: Unknown binary operand." << std::endl;
							return ConstantInt::get(getGlobalContext(), APInt(32, 0));
					}
				}
				case SEM_VALUE_TERNARY:
					std::cout << "CodeGen error: Unimplemented ternary operation." << std::endl;
					return ConstantInt::get(getGlobalContext(), APInt(32, 42));
				case SEM_VALUE_CONSTRUCT:
					std::cout << "CodeGen error: Unimplemented constructor call." << std::endl;
					return ConstantInt::get(getGlobalContext(), APInt(32, 42));
				case SEM_VALUE_MEMBERACCESS:
					std::cout << "CodeGen error: Unimplemented member access." << std::endl;
					return ConstantInt::get(getGlobalContext(), APInt(32, 42));
				case SEM_VALUE_METHODCALL:
					std::cout << "CodeGen error: Unimplemented method call." << std::endl;
					return ConstantInt::get(getGlobalContext(), APInt(32, 42));
				default:
					std::cout << "CodeGen error: Unknown value." << std::endl;
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

