#ifndef LOCIC_CODEGEN_FUNCTION_HPP
#define LOCIC_CODEGEN_FUNCTION_HPP

#include <string>

#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
	
		class ArgInfo {
			public:
				inline static ArgInfo None() {
					return ArgInfo(false, false, 0);
				}
				
				inline ArgInfo(bool hRVA, bool hCA,
							   size_t nSA)
					: hasReturnVarArgument_(hRVA),
					  hasContextArgument_(hCA),
					  numStandardArguments_(nSA) { }
					  
				bool hasReturnVarArgument() const {
					return hasReturnVarArgument_;
				}
				
				bool hasContextArgument() const {
					return hasContextArgument_;
				}
				
				size_t contextArgumentOffset() const {
					return hasReturnVarArgument() ? 1 : 0;
				}
				
				size_t standardArgumentOffset() const {
					return contextArgumentOffset() +
						   (hasContextArgument() ? 1 : 0);
				}
				
				size_t numStandardArguments() const {
					return numStandardArguments_;
				}
				
				size_t numArguments() const {
					return standardArgumentOffset() +
						   numStandardArguments();
				}
				
			private:
				bool hasReturnVarArgument_;
				bool hasContextArgument_;
				size_t numStandardArguments_;
				
		};
		
		static const std::string NO_FUNCTION_NAME = "";
		
		inline ArgInfo getArgInfo(SEM::Function* function) {
			const bool hasReturnVarArg = function->type()->getFunctionReturnType()->isClass();
			const bool hasContextArg = function->isMethod() && !function->isStatic();
			return ArgInfo(hasReturnVarArg, hasContextArg,
						   function->type()->getFunctionParameterTypes().size());
		}
		
		inline llvm::Function* createLLVMFunction(Module& module, llvm::FunctionType* type,
				llvm::GlobalValue::LinkageTypes linkage, const std::string& name) {
			return llvm::Function::Create(type, linkage, name, module.getLLVMModulePtr());
		}
		
		class Function {
			public:
				inline Function(Module& module, llvm::Function& function, const ArgInfo& argInfo)
					: module_(module), function_(function),
					  builder_(module.getLLVMContext()), argInfo_(argInfo) {
					assert(argInfo_.numArguments() == function_.type()->getNumParams());
					selectBasicBlock(createBasicBlock("entry"));
				}
				
				inline llvm::Function& getLLVMFunction() {
					return function_;
				}
				
				inline Module& getModule() {
					return module_;
				}
				
				inline const Module& getModule() const {
					return module_;
				}
				
				inline const ArgInfo& getArgInfo() const {
					return argInfo_;
				}
				
				inline llvm::Value* getRawArg(size_t index) const {
					assert(index < argInfo_.numArguments());
					llvm::Function::arg_iterator arg = function_.arg_begin();
					
					for (size_t i = 0; i < index; i++) {
						assert(arg != function_.arg_end());
						arg++;
					}
					
					assert(arg != function_->arg_end());
					return arg;
				}
				
				inline llvm::Value* getArg(size_t index) const {
					assert(index < argInfo_.numStandardArguments());
					return getRawArg(argInfo_.standardArgumentOffset() + index);
				}
				
				inline llvm::Value* getReturnVar() const {
					assert(argInfo_.hasReturnVarArgument());
					return getArg(0);
				}
				
				inline llvm::Value* getContextValue() const {
					assert(argInfo_.hasContextArgument());
					return getArg(argInfo_.contextArgumentOffset());
				}
				
				inline llvm::BasicBlock* createBasicBlock(const std::string& name) {
					return llvm::BasicBlock::Create(module_.getLLVMContext(), name, &function_);
				}
				
				inline llvm::IRBuilder<>& getBuilder() {
					return builder_;
				}
				
				inline llvm::BasicBlock* getSelectedBasicBlock() const {
					return builder_.GetInsertBlock();
				}
				
				inline void selectBasicBlock(llvm::BasicBlock* basicBlock) {
					builder_.SetInsertPoint(basicBlock);
				}
				
				inline void verify() const {
					llvm::verifyFunction(&function_);
				}
				
			private:
				Module& module_;
				llvm::Function* function_;
				llvm::IRBuilder<> builder_;
				
		}
		
	}
	
}

#endif
