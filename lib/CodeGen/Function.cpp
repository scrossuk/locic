#include <stack>
#include <string>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/Map.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
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
		
		bool isOffsetPairLessThan(const OffsetPair& first, const OffsetPair& second) {
			const bool result = compareTypes(first.first, second.first);
			
			if (result != COMPARE_EQUAL) {
				return result == COMPARE_LESS;
			}
			
			return first.second < second.second;
		}
		
		Function::Function(Module& pModule, llvm::Function& function, const ArgInfo& argInfo, TemplateBuilder* pTemplateBuilder)
			: module_(pModule), function_(function),
			  entryBuilder_(pModule.getLLVMContext()),
			  builder_(pModule.getLLVMContext()),
			  useEntryBuilder_(false),
			  argInfo_(argInfo),
			  templateBuilder_(pTemplateBuilder),
			  alignMaskMap_(isTypeLessThan),
			  memberOffsetMap_(isOffsetPairLessThan),
			  sizeOfMap_(isTypeLessThan),
			  templateGeneratorMap_(isTypeLessThan),
			  debugInfo_(nullptr),
			  landingPad_(nullptr),
			  exceptionInfo_(nullptr),
			  returnValuePtr_(nullptr),
			  templateArgs_(nullptr),
			  unwindState_(nullptr) {
			assert(function.isDeclaration());
			assert(argInfo_.numArguments() == function_.getFunctionType()->getNumParams());
			
			// Add a bottom level unwind stack.
			unwindStackStack_.push(UnwindStack());
			
			// Create an 'entry' basic block for holding
			// instructions like allocas and debug_declares
			// which must only be executed once per function.
			const auto entryBB = createBasicBlock("");
			const auto startBB = createBasicBlock("");
			
			entryBuilder_.SetInsertPoint(entryBB);
			const auto startBranch = entryBuilder_.CreateBr(startBB);
			
			// Insert entry instructions before the branch.
			entryBuilder_.SetInsertPoint(startBranch);
			
			builder_.SetInsertPoint(startBB);
			
			argValues_.reserve(function_.arg_size());
			
			for (auto arg = function_.arg_begin(); arg != function_.arg_end(); ++arg) {
				argValues_.push_back(arg);
			}
			
			std::vector<llvm_abi::Type*> argABITypes;
			argABITypes.reserve(argInfo.argumentTypes().size());
			
			std::vector<llvm::Type*> argLLVMTypes;
			argLLVMTypes.reserve(argInfo.argumentTypes().size());
			
			for (const auto& typePair : argInfo.argumentTypes()) {
				argABITypes.push_back(typePair.first);
				argLLVMTypes.push_back(typePair.second);
			}
			
			// Decode arguments according to ABI.
			module_.abi().decodeValues(getEntryBuilder(), getEntryBuilder(), argValues_, argABITypes, argLLVMTypes);
		}
		
		void Function::returnValue(llvm::Value* value) {
			assert(!argInfo_.hasReturnVarArgument());
			assert(!value->getType()->isVoidTy());
			
			// Encode return value according to ABI.
			std::vector<llvm_abi::Type*> abiTypes;
			abiTypes.push_back(getArgInfo().returnType().first);
			
			std::vector<llvm::Value*> values;
			values.push_back(value);
			module().abi().encodeValues(getEntryBuilder(), getBuilder(), values, abiTypes);
			
			getBuilder().CreateRet(values.at(0));
		}
		
		void Function::setReturnValue(llvm::Value* value) {
			assert(!argInfo_.hasReturnVarArgument());
			assert(!value->getType()->isVoidTy());
			
			// Encode return value according to ABI.
			std::vector<llvm_abi::Type*> abiTypes;
			abiTypes.push_back(getArgInfo().returnType().first);
			
			std::vector<llvm::Value*> values;
			values.push_back(value);
			module().abi().encodeValues(getEntryBuilder(), getBuilder(), values, abiTypes);
			
			const auto encodedValue = values.at(0);
			
			if (returnValuePtr_ == nullptr) {
				returnValuePtr_ = getEntryBuilder().CreateAlloca(encodedValue->getType());
			}
			
			getBuilder().CreateStore(encodedValue, returnValuePtr_);
		}
		
		llvm::Value* Function::getRawReturnValue() {
			if (argInfo_.hasReturnVarArgument() || argInfo_.returnType().second->isVoidTy()) {
				return nullptr;
			}
			
			if (returnValuePtr_ == nullptr) {
				returnValuePtr_ = getEntryBuilder().CreateAlloca(function_.getFunctionType()->getReturnType());
			}
			
			return getBuilder().CreateLoad(returnValuePtr_);
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
		
		std::vector<llvm::Value*> Function::getArgList() const {
			std::vector<llvm::Value*> argList;
			
			for (size_t i = 0; i < argInfo_.numStandardArguments(); i++) {
				argList.push_back(getArg(i));
			}
			
			return argList;
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
		
		llvm::Value* Function::getRawContextValue() const {
			assert(argInfo_.hasContextArgument());
			return getRawArg(argInfo_.contextArgumentOffset());
		}
		
		llvm::Value* Function::getContextValue(SEM::TypeInstance* typeInstance) {
			return getBuilder().CreatePointerCast(getRawContextValue(), genPointerType(module(), typeInstance->selfType()));
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
			return entryBuilder_;
		}
		
		llvm::IRBuilder<>& Function::getBuilder() {
			return useEntryBuilder_ ? entryBuilder_ : builder_;
		}
		
		bool Function::setUseEntryBuilder(bool useEntryBuilder) {
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
		
		void Function::verify() const {
			// (void) llvm::verifyFunction(function_, llvm::AbortProcessAction);
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
			landingPad_ = nullptr;
			unwindStackStack_.push(UnwindStack(unwindStack().begin(), unwindStack().begin() + position));
		}
		
		void Function::popUnwindStack() {
			landingPad_ = nullptr;
			unwindStackStack_.pop();
		}
		
		void Function::pushUnwindAction(const UnwindAction& action) {
			// TODO: only invalidate when pushed action affects exceptions.
			landingPad_ = nullptr;
			
			unwindStackStack_.top().push_back(action);
		}
		
		void Function::popUnwindAction() {
			// TODO: only invalidate when popped action affects exceptions.
			landingPad_ = nullptr;
			
			unwindStackStack_.top().pop_back();
		}
		
		const UnwindStack& Function::unwindStack() const {
			return unwindStackStack_.top();
		}
		
		llvm::LandingPadInst* Function::latestLandingPad() {
			return landingPad_;
		}
		
		void Function::setLatestLandingPad(llvm::LandingPadInst* landingPad) {
			landingPad_ = landingPad;
		}
		
		llvm::Value* Function::unwindState() {
			if (unwindState_ == nullptr) {
				const auto i8Type = TypeGenerator(module_).getI8Type();
				
				// Zero state means 'normal execution'.
				const auto zero = ConstantGenerator(module_).getI8(0);
				unwindState_ = getEntryBuilder().CreateAlloca(i8Type, nullptr, "unwindState");
				getEntryBuilder().CreateStore(zero, unwindState_);
			}
			
			return unwindState_;
		}
		
		llvm::Value* Function::exceptionInfo() {
			if (exceptionInfo_ == nullptr) {
				// Allocate exception information values.
				TypeGenerator typeGen(module());
				const auto exceptionInfoType = typeGen.getStructType(std::vector<llvm::Type*> {typeGen.getI8PtrType(), typeGen.getI32Type()});
				exceptionInfo_ = getEntryBuilder().CreateAlloca(exceptionInfoType, nullptr, "exceptionInfo");
			}
			
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

