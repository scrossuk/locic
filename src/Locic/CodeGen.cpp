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

#include <Locic/AST.h>
#include <Locic/CodeGen.h>

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
		std::map<std::string, AllocaInst *> variables_;

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
		
		void genFile(AST_Module * module){
			Locic_List * functions = module->functionDefinitions;
			for(Locic_ListElement * it = Locic_List_Begin(functions); it != Locic_List_End(functions); it = it->next){
				genFunctionDef(reinterpret_cast<AST_FunctionDef *>(it->data));
			}
		}
		
		/// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
		/// the function.  This is used for mutable variables etc.
		AllocaInst * CreateEntryBlockAlloca(const std::string &varName) {
  			IRBuilder<> tmp(&currentFunction_->getEntryBlock(),
                 		currentFunction_->getEntryBlock().begin());
			return tmp.CreateAlloca(Type::getInt32Ty(getGlobalContext()), 0, varName.c_str());
		}

		
		void genFunctionDef(AST_FunctionDef * functionDef){
			variables_.clear();
		
			currentFunctionType_ = FunctionType::get(Type::getInt32Ty(getGlobalContext()),
	                             std::vector<Type*>(Locic_List_Size(functionDef->declaration->parameters), Type::getInt32Ty(getGlobalContext())), false);
	                
	                // Create function.
			currentFunction_ = Function::Create(currentFunctionType_, Function::ExternalLinkage, functionDef->declaration->name, module_);
			
			currentBasicBlock_ = BasicBlock::Create(getGlobalContext(), "entry", currentFunction_);
			builder_.SetInsertPoint(currentBasicBlock_);
			
			// Store arguments onto stack.
			Locic_ListElement * it;
			std::size_t i;
			Function::arg_iterator arg;
			
			Locic_List * functionParameters = functionDef->declaration->parameters;
			for (it = Locic_List_Begin(functionParameters), i = 0, arg = currentFunction_->arg_begin(); it != Locic_List_End(functionParameters); ++arg, it = it->next, i++){
				AST_TypeVar * typeVar = reinterpret_cast<AST_TypeVar *>(it->data);
				
				// Create an alloca for this variable.
    				AllocaInst * stackObject = CreateEntryBlockAlloca(typeVar->name);
    				
    				variables_[std::string(typeVar->name)] = stackObject;

    				// Store the initial value into the alloca.
    				builder_.CreateStore(arg, stackObject);
			}
			
			genScope(functionDef->scope);
			
			verifyFunction(*currentFunction_);
			
			fpm_.run(*currentFunction_);
		}
		
		void genScope(AST_Scope * scope){
			Locic_List * list = scope->statementList;
			
			for(Locic_ListElement * it = Locic_List_Begin(list); it != Locic_List_End(list); it = it->next){
				AST_Statement * statement = reinterpret_cast<AST_Statement *>(it->data);
				genStatement(statement);
			}
		}
		
		void genStatement(AST_Statement * statement){
			switch(statement->type){
				case AST_STATEMENT_VALUE:
					
					break;
				case AST_STATEMENT_IF:
					
					break;
				case AST_STATEMENT_VARDECL:
				{
					// Create an alloca for this variable.
    					AllocaInst * stackObject = CreateEntryBlockAlloca(statement->varDecl.varName);
    				
    					variables_[std::string(statement->varDecl.varName)] = stackObject;

    					// Store the initial value into the alloca.
    					builder_.CreateStore(genValue(statement->varDecl.value), stackObject);
					break;
				}
				case AST_STATEMENT_ASSIGNVAR:
				{
					AST_Var * var = statement->assignVar.var;
					switch(var->type){
						case AST_VAR_LOCAL:
							builder_.CreateStore(genValue(statement->assignVar.value), variables_[std::string(var->localVar.name)]);
							break;
						case AST_VAR_THIS:
							break;
						default:
							std::cout << "Unknown variable type" << std::endl;
					}
					break;
				}
				case AST_STATEMENT_RETURN:
					builder_.CreateRet(genValue(statement->returnStmt.value));
					break;
				default:
					std::cout << "Unknown statement" << std::endl;
			}
		}
		
		Value * genValue(AST_Value * value){
			switch(value->type){
				case AST_VALUE_CONSTANT:
				{
					switch(value->constant.type){
						case AST_CONSTANT_BOOL:
							return ConstantInt::get(getGlobalContext(), APInt(32, value->constant.boolConstant));
						case AST_CONSTANT_INT:
							return ConstantInt::get(getGlobalContext(), APInt(32, value->constant.intConstant));
						case AST_CONSTANT_FLOAT:
							return ConstantInt::get(getGlobalContext(), APInt(32, 42));
						default:
							std::cout << "Unknown constant" << std::endl;
							return ConstantInt::get(getGlobalContext(), APInt(32, 0));
					}
				}
				case AST_VALUE_VARACCESS:
				{
					AST_Var * var = value->varAccess.var;
					switch(var->type){
						case AST_VAR_LOCAL:
							return builder_.CreateLoad(variables_[std::string(var->localVar.name)], var->localVar.name);
						case AST_VAR_THIS:
							return ConstantInt::get(getGlobalContext(), APInt(32, 1));
						default:
							std::cout << "Unknown variable type" << std::endl;
							return ConstantInt::get(getGlobalContext(), APInt(32, 0));
					}
					return ConstantInt::get(getGlobalContext(), APInt(32, 1));
				}
				case AST_VALUE_UNARY:
				{
					return ConstantInt::get(getGlobalContext(), APInt(32, 2));
				}
				case AST_VALUE_BINARY:
				{
					switch(value->binary.type){
						case AST_BINARY_ADD:
							return builder_.CreateAdd(genValue(value->binary.left), genValue(value->binary.right));
						case AST_BINARY_SUBTRACT:
							return builder_.CreateSub(genValue(value->binary.left), genValue(value->binary.right));
						case AST_BINARY_MULTIPLY:
							return builder_.CreateMul(genValue(value->binary.left), genValue(value->binary.right));
						case AST_BINARY_DIVIDE:
							return builder_.CreateSDiv(genValue(value->binary.left), genValue(value->binary.right));
						case AST_BINARY_ISEQUAL:
							return builder_.CreateICmpEQ(genValue(value->binary.left), genValue(value->binary.right));
						case AST_BINARY_NOTEQUAL:
							return builder_.CreateICmpNE(genValue(value->binary.left), genValue(value->binary.right));
						case AST_BINARY_GREATEROREQUAL:
							return builder_.CreateICmpSGE(genValue(value->binary.left), genValue(value->binary.right));
						case AST_BINARY_LESSOREQUAL:
							return builder_.CreateICmpSLE(genValue(value->binary.left), genValue(value->binary.right));
						default:
							std::cout << "Unknown binary operand" << std::endl;
							return ConstantInt::get(getGlobalContext(), APInt(32, 0));
					}
				}
				case AST_VALUE_TERNARY:
				case AST_VALUE_CONSTRUCT:
				case AST_VALUE_MEMBERACCESS:
				case AST_VALUE_METHODCALL:
					return ConstantInt::get(getGlobalContext(), APInt(32, 42));
				default:
					std::cout << "Unknown value" << std::endl;
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
	
	void Locic_CodeGen(void * context, AST_Module * module){
		reinterpret_cast<CodeGen *>(context)->genFile(module);
	}
	
	void Locic_CodeGenDump(void * context){
		reinterpret_cast<CodeGen *>(context)->dump();
	}
	
}

