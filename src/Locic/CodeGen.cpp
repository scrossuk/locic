#include <llvm/DerivedTypes.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <cstdio>
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

	public:
		CodeGen(const char * moduleName) : name_(moduleName), builder_(getGlobalContext()){
			InitializeNativeTarget();
			LLVMContext& context = getGlobalContext();
			module_ = new Module(name_.c_str(), context);
		}
		
		~CodeGen(){
			delete module_;
		}
		
		void dump(){
			module_->dump();
		}
		
		void genFile(AST_File * file){
			AST_List * functions = file->functionDefinitions;
			for(AST_ListElement * it = AST_ListBegin(functions); it != AST_ListEnd(); it = it->next){
				genFunctionDef(reinterpret_cast<AST_FunctionDef *>(it->data));
			}
		}
		
		void genFunctionDef(AST_FunctionDef * functionDef){
			currentFunctionType_ = FunctionType::get(Type::getInt32Ty(getGlobalContext()),
	                             std::vector<Type*>(), false);
	                             
			currentFunction_ = Function::Create(currentFunctionType_, Function::ExternalLinkage, functionDef->declaration->name, module_);
			
			BasicBlock* basicBlock = BasicBlock::Create(getGlobalContext(), "entry", currentFunction_);
			builder_.SetInsertPoint(basicBlock);
			
			builder_.CreateRet(ConstantInt::get(getGlobalContext(), APInt(32, 42)));
			
			verifyFunction(*currentFunction_);
		}
	
};

extern "C"{
	
	void * Locic_CodeGenAlloc(const char * moduleName){
		return new CodeGen(moduleName);
	}
	
	void Locic_CodeGenFree(void * context){
		delete reinterpret_cast<CodeGen *>(context);
	}
	
	void Locic_CodeGen(void * context, AST_File * file){
		reinterpret_cast<CodeGen *>(context)->genFile(file);
	}
	
	void Locic_CodeGenDump(void * context){
		reinterpret_cast<CodeGen *>(context)->dump();
	}
	
}

