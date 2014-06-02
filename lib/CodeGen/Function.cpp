#include <stack>
#include <string>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/Map.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Function* createLLVMFunction(Module& module, llvm::FunctionType* type,
				llvm::GlobalValue::LinkageTypes linkage, const std::string& name) {
			return llvm::Function::Create(type, linkage, name, module.getLLVMModulePtr());
		}
		
		Function::Function(Module& pModule, llvm::Function& function, ArgInfo argInfo)
			: module_(pModule), function_(function),
			  entryBuilder_(pModule.getLLVMContext()),
			  builder_(pModule.getLLVMContext()),
			  argInfo_(std::move(argInfo)),
			  exceptionInfo_(nullptr),
			  debugInfo_(nullptr),
			  templateArgs_(nullptr) {
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
			const auto exceptionInfoType = typeGen.getStructType(std::vector<llvm::Type*> {typeGen.getI8PtrType(), typeGen.getI32Type()});
			exceptionInfo_ = getEntryBuilder().CreateAlloca(exceptionInfoType, nullptr, "exceptionInfo");
			
			// Decode arguments according to ABI.
			std::vector<llvm::Value*> encodedArgValues;
			
			for (auto arg = function_.arg_begin(); arg != function_.arg_end(); ++arg) {
				encodedArgValues.push_back(arg);
			}
			
			argValues_ = module_.abi().decodeValues(getEntryBuilder(), getBuilder(), encodedArgValues, argInfo_.abiTypes(), argInfo_.abiLLVMTypes());
			
			// Obtain template arguments (if applicable).
			if (argInfo_.hasTemplateGeneratorArgument()) {
				templateArgs_ = computeTemplateArguments(*this, getRawArg(argInfo_.templateGeneratorArgumentOffset()));
			}
		}
		
		llvm::Function& Function::getLLVMFunction() {
			return function_;
		}
		
		llvm::Function* Function::getLLVMFunctionPtr() {
			return &function_;
		}
		
		Module& Function::module() {
			return module_;
		}
		
		const Module& Function::module() const {
			return module_;
		}
		
		const ArgInfo& Function::getArgInfo() const {
			return argInfo_;
		}
		
		llvm::Value* Function::getRawArg(size_t index) const {
			assert(index < argInfo_.numArguments());
			return argValues_.at(index);
		}
		
		llvm::Value* Function::getArg(size_t index) const {
			assert(index < argInfo_.numStandardArguments());
			return getRawArg(argInfo_.standardArgumentOffset() + index);
		}
		
		llvm::Value* Function::getTemplateGenerator() const {
			assert(argInfo_.hasTemplateGeneratorArgument());
			return getRawArg(argInfo_.templateGeneratorArgumentOffset());
		}
		
		llvm::Value* Function::getTemplateArgs() const {
			assert(templateArgs_ != nullptr);
			return templateArgs_;
		}
		
		llvm::Value* Function::getReturnVar() const {
			assert(argInfo_.hasReturnVarArgument());
			return getRawArg(argInfo_.returnVarArgumentOffset());
		}
		
		llvm::Value* Function::getContextValue() const {
			assert(argInfo_.hasContextArgument());
			return getRawArg(argInfo_.contextArgumentOffset());
		}
		
		llvm::BasicBlock* Function::createBasicBlock(const std::string& name) {
			return llvm::BasicBlock::Create(module_.getLLVMContext(), name, &function_);
		}
		
		// Returns an 'entry' builder for creating instructions
		// in the first ('entry') basic block.
		llvm::IRBuilder<>& Function::getEntryBuilder() {
			return entryBuilder_;
		}
		
		llvm::IRBuilder<>& Function::getBuilder() {
			return builder_;
		}
		
		llvm::BasicBlock* Function::getSelectedBasicBlock() const {
			return builder_.GetInsertBlock();
		}
		
		void Function::selectBasicBlock(llvm::BasicBlock* basicBlock) {
			builder_.SetInsertPoint(basicBlock);
		}
		
		void Function::verify() const {
			(void) llvm::verifyFunction(function_, llvm::AbortProcessAction);
		}
		
		Function::LocalVarMap& Function::getLocalVarMap() {
			return localVarMap_;
		}
		
		const Function::LocalVarMap& Function::getLocalVarMap() const {
			return localVarMap_;
		}
		
		void Function::pushUnwindStack(size_t position) {
			unwindStackStack_.push(UnwindStack(unwindStack().begin(), unwindStack().begin() + position));
		}
		
		void Function::popUnwindStack() {
			unwindStackStack_.pop();
		}
		
		UnwindStack& Function::unwindStack() {
			return unwindStackStack_.top();
		}
		
		const UnwindStack& Function::unwindStack() const {
			return unwindStackStack_.top();
		}
		
		llvm::Value* Function::exceptionInfo() const {
			return exceptionInfo_;
		}
		
		void Function::attachDebugInfo(llvm::DISubprogram subprogram) {
			debugInfo_ = subprogram;
		}
		
		llvm::DISubprogram Function::debugInfo() const {
			return debugInfo_;
		}
		
	}
	
}

