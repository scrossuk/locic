#ifndef LOCIC_CODEGEN_FUNCTION_HPP
#define LOCIC_CODEGEN_FUNCTION_HPP

#include <stack>
#include <string>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/Map.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		static const std::string NO_FUNCTION_NAME = "";
		
		inline llvm::Function* createLLVMFunction(Module& module, llvm::FunctionType* type,
				llvm::GlobalValue::LinkageTypes linkage, const std::string& name) {
			return llvm::Function::Create(type, linkage, name, module.getLLVMModulePtr());
		}
		
		// TODO: move method implementations to source file.
		class Function {
			public:
				typedef Map<SEM::Var*, llvm::Value*> LocalVarMap;
				
				inline Function(Module& pModule, llvm::Function& function, ArgInfo argInfo)
					: module_(pModule), function_(function),
					  entryBuilder_(pModule.getLLVMContext()),
					  builder_(pModule.getLLVMContext()),
					  argInfo_(std::move(argInfo)),
					  exceptionInfo_(nullptr), debugInfo_(nullptr) {
					assert(function.isDeclaration());
					assert(argInfo_.numArguments() == function_.getFunctionType()->getNumParams());
					
					// Add a bottom level unwind stack.
					unwindStackStack_.push(UnwindStack());
					
					// Create an 'entry' basic block for holding
					// instructions like allocas and debug_declares
					// which must only be executed once per function.
					const auto entryBB = createBasicBlock("");
					const auto startBB = createBasicBlock("start");
					
					entryBuilder_.SetInsertPoint(entryBB);
					const auto startBranch = entryBuilder_.CreateBr(startBB);
					
					// Insert entry instructions before the branch.
					entryBuilder_.SetInsertPoint(startBranch);
					
					builder_.SetInsertPoint(startBB);
					
					// Allocate exception information values.
					TypeGenerator typeGen(pModule);
					const auto exceptionInfoType = typeGen.getStructType(std::vector<llvm::Type*>{typeGen.getI8PtrType(), typeGen.getI32Type()});
					exceptionInfo_ = getEntryBuilder().CreateAlloca(exceptionInfoType, nullptr, "exceptionInfo");
					
					std::vector<llvm::Value*> encodedArgValues;
					
					for (auto arg = function_.arg_begin(); arg != function_.arg_end(); ++arg) {
						encodedArgValues.push_back(arg);
					}
					
					argValues_ = module_.abi().decodeValues(getEntryBuilder(), getBuilder(), encodedArgValues, argInfo_.abiTypes(), argInfo_.abiLLVMTypes());
				}
				
				inline llvm::Function& getLLVMFunction() {
					return function_;
				}
				
				inline llvm::Function* getLLVMFunctionPtr() {
					return &function_;
				}
				
				inline Module& module() {
					return module_;
				}
				
				inline const Module& module() const {
					return module_;
				}
				
				inline const ArgInfo& getArgInfo() const {
					return argInfo_;
				}
				
				inline llvm::Value* getRawArg(size_t index) const {
					assert(index < argInfo_.numArguments());
					return argValues_.at(index);
				}
				
				inline llvm::Value* getArg(size_t index) const {
					assert(index < argInfo_.numStandardArguments());
					return getRawArg(argInfo_.standardArgumentOffset() + index);
				}
				
				inline llvm::Value* getTemplateGenerator() const {
					assert(argInfo_.hasTemplateGeneratorArgument());
					return getRawArg(argInfo_.templateGeneratorArgumentOffset());
				}
				
				inline llvm::Value* getReturnVar() const {
					assert(argInfo_.hasReturnVarArgument());
					return getRawArg(argInfo_.returnVarArgumentOffset());
				}
				
				inline llvm::Value* getContextValue() const {
					assert(argInfo_.hasContextArgument());
					return getRawArg(argInfo_.contextArgumentOffset());
				}
				
				inline llvm::BasicBlock* createBasicBlock(const std::string& name) {
					return llvm::BasicBlock::Create(module_.getLLVMContext(), name, &function_);
				}
				
				// Returns an 'entry' builder for creating instructions
				// in the first ('entry') basic block.
				inline llvm::IRBuilder<>& getEntryBuilder() {
					return entryBuilder_;
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
					(void) llvm::verifyFunction(function_, llvm::AbortProcessAction);
				}
				
				inline LocalVarMap& getLocalVarMap() {
					return localVarMap_;
				}
				
				inline const LocalVarMap& getLocalVarMap() const {
					return localVarMap_;
				}
				
				inline void pushUnwindStack(size_t position) {
					unwindStackStack_.push(UnwindStack(unwindStack().begin(), unwindStack().begin() + position));
				}
				
				inline void popUnwindStack() {
					unwindStackStack_.pop();
				}
				
				inline UnwindStack& unwindStack() {
					return unwindStackStack_.top();
				}
				
				inline const UnwindStack& unwindStack() const {
					return unwindStackStack_.top();
				}
				
				inline llvm::Value* exceptionInfo() const {
					return exceptionInfo_;
				}
				
				inline void attachDebugInfo(llvm::DISubprogram subprogram) {
					debugInfo_ = subprogram;
				}
				
				inline llvm::DISubprogram debugInfo() const {
					return debugInfo_;
				}
				
			private:
				Module& module_;
				llvm::Function& function_;
				llvm::IRBuilder<> entryBuilder_, builder_;
				ArgInfo argInfo_;
				LocalVarMap localVarMap_;
				
				// A 'stack' of unwind stacks.
				std::stack<UnwindStack> unwindStackStack_;
				
				llvm::Value* exceptionInfo_;
				llvm::DISubprogram debugInfo_;
				std::vector<llvm::Value*> argValues_;
				
		};
		
	}
	
}

#endif
