#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>

#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/Support/Map.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Function* createLLVMFunction(Module& module, const ArgInfo& argInfo, llvm::GlobalValue::LinkageTypes linkage, const String& name) {
			const auto llvmFunction = llvm::Function::Create(argInfo.makeFunctionType(), linkage, name.c_str(), module.getLLVMModulePtr());
			
			if (argInfo.noMemoryAccess()) {
				llvmFunction->setDoesNotAccessMemory();
			}
			
			if (argInfo.noExcept()) {
				llvmFunction->setDoesNotThrow();
			}
			
			if (argInfo.noReturn()) {
				llvmFunction->setDoesNotReturn();
			}
			
			if (argInfo.hasNestArgument()) {
				llvmFunction->addAttribute(1, llvm::Attribute::Nest);
			}
			
			const auto abiFunctionType = argInfo.getABIFunctionType();
			const auto attributes = module.abi().getAttributes(abiFunctionType,
			                                                   abiFunctionType.argumentTypes(),
			                                                   llvmFunction->getAttributes());
			llvmFunction->setAttributes(attributes);
			
			return llvmFunction;
		}
		
		Function::Function(Module& pModule, llvm::Function& function, const ArgInfo& argInfo, TemplateBuilder* pTemplateBuilder)
			: module_(pModule), function_(function),
			  entryBuilder_(pModule.getLLVMContext()),
			  builder_(pModule.getLLVMContext()),
			  createdEntryBlock_(false),
			  useEntryBuilder_(false),
			  argInfo_(argInfo),
			  templateBuilder_(pTemplateBuilder),
			  debugInfo_(nullptr),
#if LOCIC_LLVM_VERSION < 307
			  personalityFunction_(nullptr),
#endif
			  exceptionInfo_(nullptr),
			  returnValuePtr_(nullptr),
			  templateArgs_(nullptr),
			  unwindState_(nullptr) {
			assert(function.isDeclaration());
			
			// Add a bottom level unwind stack.
			unwindStackStack_.push(UnwindStack());
			
			// Add bottom level action for this function.
			unwindStack().push_back(UnwindAction::FunctionMarker());
			
			const auto startBB = createBasicBlock("");
			builder_.SetInsertPoint(startBB);
			
			llvm::SmallVector<llvm::Value*, 8> argValues;
			argValues.reserve(function_.arg_size());
			
			for (auto arg = function_.arg_begin(); arg != function_.arg_end(); ++arg) {
				argValues.push_back(&*arg);
			}
			
			SetUseEntryBuilder useEntryBuilder(*this);
			
			// Decode arguments according to ABI.
			functionEncoder_ = module().abi().createFunctionEncoder(/*builder=*/*this,
			                                                        argInfo.getABIFunctionType(),
			                                                        argValues);
		}
		
		llvm::Instruction* Function::returnValue(llvm::Value* value) {
			assert(!argInfo_.hasReturnVarArgument());
			assert(!value->getType()->isVoidTy());
			
			if (value->getType()->isPointerTy()) {
				value = getBuilder().CreatePointerCast(value,
				                                       TypeGenerator(module()).getPtrType());
			}
			
			return functionEncoder_->returnValue(value);
		}
		
		void Function::setReturnValue(llvm::Value* const value) {
			assert(!argInfo_.hasReturnVarArgument());
			assert(!value->getType()->isVoidTy());
			
			if (returnValuePtr_ == nullptr) {
				returnValuePtr_ = getEntryBuilder().CreateAlloca(argInfo_.returnType().second,
				                                                 nullptr, "returnvalueptr");
			}
			
			IREmitter irEmitter(*this);
			irEmitter.emitRawStore(value, returnValuePtr_);
		}
		
		llvm::Value* Function::getRawReturnValue() {
			if (argInfo_.hasReturnVarArgument() || argInfo_.returnType().second->isVoidTy()) {
				return nullptr;
			}
			
			if (returnValuePtr_ == nullptr) {
				returnValuePtr_ = getEntryBuilder().CreateAlloca(argInfo_.returnType().second,
				                                                 nullptr, "returnvalueptr");
			}
			
			IREmitter irEmitter(*this);
			return irEmitter.emitRawLoad(returnValuePtr_,
			                             argInfo_.returnType().second);
		}
		
		llvm::Value* Function::getVarAddress(const AST::Var& var) {
			return localVarMap_.get(&var);
		}
		
		void Function::setVarAddress(const AST::Var& var,
		                             llvm::Value* const varPtr) {
			localVarMap_.insert(&var, varPtr);
			
			if (!var.isPattern()) return;
			
			IREmitter irEmitter(*this);
			
			// For composite variables, set the pointer of each
			// variable to the relevant offset in the object.
			for (size_t i = 0; i < var.varList()->size(); i++) {
				const auto& childVar = (*(var.varList()))[i];
				const auto memberOffsetValue = genMemberOffset(*this, var.constructType(), i);
				const auto memberPtr = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
				                                                 varPtr, memberOffsetValue);
				setVarAddress(*childVar, memberPtr);
			}
		}
		
		llvm::Function& Function::getLLVMFunction() {
			return function_;
		}
		
		const llvm::Function& Function::getLLVMFunction() const {
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
			return functionEncoder_->arguments()[index];
		}
		
		llvm::Value* Function::getArg(size_t index) const {
			assert(index < argInfo_.numStandardArguments());
			return getRawArg(argInfo_.standardArgumentOffset() + index);
		}
		
		std::vector<llvm::Value*> Function::getArgList() const {
			std::vector<llvm::Value*> argList;
			
			for (size_t i = 0; i < argInfo_.numStandardArguments(); i++) {
				argList.push_back(getArg(i));
			}
			
			return argList;
		}
		
		llvm::Value* Function::getNestArgument() const {
			assert(argInfo_.hasNestArgument());
			return getRawArg(argInfo_.nestArgumentOffset());
		}
		
		llvm::Value* Function::getTemplateGenerator() const {
			assert(argInfo_.hasTemplateGeneratorArgument());
			return getRawArg(argInfo_.templateGeneratorArgumentOffset());
		}
		
		llvm::Value* Function::getTemplateArgs() {
			assert(argInfo_.hasTemplateGeneratorArgument());
			
			if (templateArgs_ == nullptr) {
				SetUseEntryBuilder useEntryBuilder(*this);
				templateArgs_ = computeTemplateArguments(*this, getRawArg(argInfo_.templateGeneratorArgumentOffset()));
			}
			
			return templateArgs_;
		}
		
		llvm::Value* Function::getReturnVar() const {
			assert(argInfo_.hasReturnVarArgument());
			return getRawArg(argInfo_.returnVarArgumentOffset());
		}
		
		llvm::Value* Function::getReturnVarOrNull() const {
			if (argInfo_.hasReturnVarArgument()) {
				return getReturnVar();
			} else {
				return nullptr;
			}
		}
		
		llvm::Value* Function::getContextValue() const {
			assert(argInfo_.hasContextArgument());
			return getRawArg(argInfo_.contextArgumentOffset());
		}
		
		TemplateBuilder& Function::templateBuilder() {
			assert(templateBuilder_ != nullptr);
			return *templateBuilder_;
		}
		
		llvm::BasicBlock* Function::createBasicBlock(const std::string& name) {
			return llvm::BasicBlock::Create(module_.getLLVMContext(), name, &function_);
		}
		
		// Returns an 'entry' builder for creating instructions
		// in the first ('entry') basic block.
		llvm::IRBuilder<>& Function::getEntryBuilder() {
			if (!createdEntryBlock_) {
				const auto startBB = &(function_.getEntryBlock());
				
				// Create an 'entry' basic block for holding
				// instructions like allocas and debug_declares
				// which must only be executed once per function.
				const auto entryBB = llvm::BasicBlock::Create(module_.getLLVMContext(), "", &function_, startBB);
				entryBuilder_.SetInsertPoint(entryBB);
				
				// Create a branch to the first code block.
				const auto startBranch = entryBuilder_.CreateBr(startBB);
				
				// Insert entry instructions before the branch.
				entryBuilder_.SetInsertPoint(startBranch);
				
				createdEntryBlock_ = true;
			}
			
			// Update debug location.
			entryBuilder_.SetCurrentDebugLocation(builder_.getCurrentDebugLocation());
			
			return entryBuilder_;
		}
		
		const llvm::IRBuilder<>& Function::getEntryBuilder() const {
			return const_cast<Function&>(*this).getEntryBuilder();
		}
		
		llvm::IRBuilder<>& Function::getBuilder() {
			return useEntryBuilder_ ? getEntryBuilder() : builder_;
		}
		
		const llvm::IRBuilder<>& Function::getBuilder() const {
			return useEntryBuilder_ ? getEntryBuilder() : builder_;
		}
		
		bool Function::setUseEntryBuilder(const bool useEntryBuilder) {
			const bool previous = useEntryBuilder_;
			useEntryBuilder_ = useEntryBuilder;
			return previous;
		}
		
		llvm::BasicBlock* Function::getSelectedBasicBlock() const {
			return builder_.GetInsertBlock();
		}
		
		void Function::selectBasicBlock(llvm::BasicBlock* basicBlock) {
			builder_.SetInsertPoint(basicBlock);
		}
		
		bool Function::lastInstructionTerminates() const {
			if (!getBuilder().GetInsertBlock()->empty()) {
				auto iterator = getBuilder().GetInsertPoint();
				--iterator;
				return iterator->isTerminator();
			} else {
				return false;
			}
		}
		
		void Function::verify() const {
			// Only verify functions when built in debug mode.
#if !defined(NDEBUG)
#if LOCIC_LLVM_VERSION >= 306 || LOCIC_LLVM_VERSION <= 308
			// This causes lots of failures in LLVM 3.6 to 3.8 due to debugging information.
			// TODO: find a way to fix this (if possible).
#elif LOCIC_LLVM_VERSION >= 305
			llvm::raw_os_ostream cerrStream(std::cerr);
			const bool result = llvm::verifyFunction(function_, &cerrStream);
			if (result)
			{
				const std::string functionName = getLLVMFunction().getName();
				getLLVMFunction().dump();
				throw std::runtime_error(makeString("Verification failed for function '%s'.", functionName.c_str()));
			}
#else
			(void) llvm::verifyFunction(function_, llvm::AbortProcessAction);
#endif
#endif
		}
		
		AlignMaskMap& Function::alignMaskMap() {
			return alignMaskMap_;
		}
		
		LocalVarMap& Function::getLocalVarMap() {
			return localVarMap_;
		}
		
		MemberOffsetMap& Function::getMemberOffsetMap() {
			return memberOffsetMap_;
		}
		
		SizeOfMap& Function::sizeOfMap() {
			return sizeOfMap_;
		}
		
		TemplateGeneratorMap& Function::templateGeneratorMap() {
			return templateGeneratorMap_;
		}
		
		void Function::pushUnwindStack(size_t position) {
			// Position needs to include top level function action.
			assert(position >= 1);
			unwindStackStack_.push(UnwindStack(unwindStack().begin(), unwindStack().begin() + position));
		}
		
		void Function::popUnwindStack() {
			unwindStackStack_.pop();
		}
		
		void Function::pushUnwindAction(const UnwindAction& action) {
			unwindStackStack_.top().push_back(action);
		}
		
		void Function::popUnwindAction() {
			unwindStackStack_.top().pop_back();
		}
		
		UnwindStack& Function::unwindStack() {
			return unwindStackStack_.top();
		}
		
		llvm::Value* Function::unwindState() {
			if (unwindState_ == nullptr) {
				const auto i8Type = TypeGenerator(module_).getI8Type();
				
				// Zero state means 'normal execution'.
				const auto zero = ConstantGenerator(module_).getI8(0);
				unwindState_ = getEntryBuilder().CreateAlloca(i8Type, nullptr, "unwindState");
				
				SetUseEntryBuilder useEntryBuilder(*this);
				IREmitter irEmitter(*this);
				irEmitter.emitRawStore(zero, unwindState_);
			}
			
			return unwindState_;
		}
		
		llvm::Value* Function::exceptionInfo() {
			if (exceptionInfo_ == nullptr) {
				// Allocate exception information values.
				TypeGenerator typeGen(module());
				const auto exceptionInfoType = typeGen.getStructType(std::vector<llvm::Type*> {typeGen.getPtrType(), typeGen.getI32Type()});
				exceptionInfo_ = getEntryBuilder().CreateAlloca(exceptionInfoType, nullptr, "exceptionInfo");
			}
			
			return exceptionInfo_;
		}
		
		void Function::attachDebugInfo(const DISubprogram subprogram) {
			debugInfo_ = subprogram;
#if LOCIC_LLVM_VERSION >= 308
			function_.setSubprogram(subprogram);
#endif
		}
		
		DISubprogram Function::debugInfo() const {
			assert(debugInfo_ != nullptr);
			return debugInfo_;
		}
		
		void Function::setDebugPosition(const Debug::SourcePosition& position) {
			const auto debugLoc = llvm::DebugLoc::get(position.lineNumber(), position.column(), debugInfo());
			getBuilder().SetCurrentDebugLocation(debugLoc);
		}
		
		llvm::DebugLoc Function::getDebugLoc() const {
			return builder_.getCurrentDebugLocation();
		}
		
		void Function::setPersonalityFunction(llvm::Constant* const argPersonalityFunction) {
#if LOCIC_LLVM_VERSION >= 307
			function_.setPersonalityFn(argPersonalityFunction);
#else
			personalityFunction_ = argPersonalityFunction;
#endif
		}
		
		llvm::Constant* Function::personalityFunction() const {
#if LOCIC_LLVM_VERSION >= 307
			return function_.getPersonalityFn();
#else
			return personalityFunction_;
#endif
		}
		
	}
	
}

