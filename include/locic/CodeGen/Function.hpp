#ifndef LOCIC_CODEGEN_FUNCTION_HPP
#define LOCIC_CODEGEN_FUNCTION_HPP

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
		
		typedef std::vector<UnwindAction> UnwindStack;
		
		// TODO: move method implementations to source file.
		class Function {
			public:
				typedef Map<SEM::Var*, llvm::Value*> LocalVarMap;
				
				inline Function(Module& module, llvm::Function& function, const ArgInfo& argInfo)
					: module_(module), function_(function),
					  builder_(module.getLLVMContext()), argInfo_(argInfo),
					  exceptionInfo_(nullptr) {
					assert(function.isDeclaration());
					assert(argInfo_.numArguments() == function_.getFunctionType()->getNumParams());
					selectBasicBlock(createBasicBlock("entry"));
					
					// Allocate exception information values.
					TypeGenerator typeGen(module);
					const auto exceptionInfoType = typeGen.getStructType(std::vector<llvm::Type*>{typeGen.getI8PtrType(), typeGen.getI32Type()});
					exceptionInfo_ = getBuilder().CreateAlloca(exceptionInfoType, nullptr, "exceptionInfo");
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
					
					assert(arg != function_.arg_end());
					return arg;
				}
				
				inline llvm::Value* getArg(size_t index) const {
					assert(index < argInfo_.numStandardArguments());
					return getRawArg(argInfo_.standardArgumentOffset() + index);
				}
				
				inline llvm::Value* getReturnVar() const {
					assert(argInfo_.hasReturnVarArgument());
					return getRawArg(0);
				}
				
				inline llvm::Value* getContextValue() const {
					assert(argInfo_.hasContextArgument());
					return getRawArg(argInfo_.contextArgumentOffset());
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
					(void) llvm::verifyFunction(function_, llvm::AbortProcessAction);
				}
				
				inline LocalVarMap& getLocalVarMap() {
					return localVarMap_;
				}
				
				inline const LocalVarMap& getLocalVarMap() const {
					return localVarMap_;
				}
				
				inline UnwindStack& unwindStack() {
					return unwindStack_;
				}
				
				inline const UnwindStack& unwindStack() const {
					return unwindStack_;
				}
				
				llvm::Value* exceptionInfo() const {
					return exceptionInfo_;
				}
				
			private:
				Module& module_;
				llvm::Function& function_;
				llvm::IRBuilder<> builder_;
				ArgInfo argInfo_;
				LocalVarMap localVarMap_;
				UnwindStack unwindStack_;
				llvm::Value* exceptionInfo_;
				
		};
		
	}
	
}

#endif
