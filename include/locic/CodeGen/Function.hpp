#ifndef LOCIC_CODEGEN_FUNCTION_HPP
#define LOCIC_CODEGEN_FUNCTION_HPP

#include <map>
#include <stack>
#include <string>
#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/Map.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TemplateBuilder.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		static const std::string NO_FUNCTION_NAME = "";
		
		llvm::Function* createLLVMFunction(Module& module, llvm::FunctionType* type,
										   llvm::GlobalValue::LinkageTypes linkage, const std::string& name);
										   
		typedef std::map<SEM::Type*, llvm::Value*, bool(*)(SEM::Type*, SEM::Type*)> AlignMaskMap;
		typedef Map<SEM::Var*, llvm::Value*> LocalVarMap;
		typedef std::pair<SEM::Type*, size_t> OffsetPair;
		typedef std::map<OffsetPair, llvm::Value*, bool(*)(const OffsetPair&, const OffsetPair&)> MemberOffsetMap;
		typedef std::map<SEM::Type*, llvm::Value*, bool(*)(SEM::Type*, SEM::Type*)> SizeOfMap;
		typedef std::map<SEM::Type*, llvm::Value*, bool(*)(SEM::Type*, SEM::Type*)> TemplateGeneratorMap;
		typedef std::vector<UnwindAction> UnwindStack;
		
		class Function {
			public:
				Function(Module& pModule, llvm::Function& function, const ArgInfo& argInfo, TemplateBuilder* templateBuilder = nullptr);
				
				void returnValue(llvm::Value* value);
				
				void setReturnValue(llvm::Value* value);
				
				llvm::Value* getRawReturnValue();
				
				llvm::Function& getLLVMFunction();
				
				llvm::Function* getLLVMFunctionPtr();
				
				Module& module();
				
				const Module& module() const;
				
				const ArgInfo& getArgInfo() const;
				
				llvm::Value* getRawArg(size_t index) const;
				
				llvm::Value* getArg(size_t index) const;
				
				std::vector<llvm::Value*> getArgList() const;
				
				llvm::Value* getTemplateGenerator() const;
				
				llvm::Value* getTemplateArgs();
				
				llvm::Value* getReturnVar() const;
				
				llvm::Value* getRawContextValue() const;
				
				llvm::Value* getContextValue(SEM::TypeInstance* typeInstance);
				
				TemplateBuilder& templateBuilder();
				
				llvm::BasicBlock* createBasicBlock(const std::string& name);
				
				// Returns an 'entry' builder for creating instructions
				// in the first ('entry') basic block.
				llvm::IRBuilder<>& getEntryBuilder();
				
				llvm::IRBuilder<>& getBuilder();
				
				bool setUseEntryBuilder(bool useEntryBuilder);
				
				llvm::BasicBlock* getSelectedBasicBlock() const;
				
				void selectBasicBlock(llvm::BasicBlock* basicBlock);
				
				void verify() const;
				
				AlignMaskMap& alignMaskMap();
				
				LocalVarMap& getLocalVarMap();
				
				MemberOffsetMap& getMemberOffsetMap();
				
				SizeOfMap& sizeOfMap();
				
				TemplateGeneratorMap& templateGeneratorMap();
				
				/**
				 * \brief Push a new unwind stack on the stack of unwind stacks.
				 *
				 * This will copy the top unwind stack up to the position
				 * specified to a new unwind stack which is then pushed on
				 * to the stack of unwind stacks.
				 *
				 * This is used for scope exit actions, since they need a new
				 * partial unwind stack when their code is being generated,
				 * since a scope(success) block is allowed to throw.
				 */
				void pushUnwindStack(size_t position);
				
				/**
				 * \brief Pop an unwind stack previous pushed.
				 */
				void popUnwindStack();
				
				void pushUnwindAction(const UnwindAction& action);
				
				void popUnwindAction();
				
				UnwindStack& unwindStack();
				
				// Value to determine the state during unwinding.
				llvm::Value* unwindState();
				
				// Value to determine information about an exception
				// currently being handled.
				llvm::Value* exceptionInfo();
				
				void attachDebugInfo(llvm::DISubprogram subprogram);
				
				llvm::DISubprogram debugInfo() const;
				
			private:
				// Non-copyable.
				Function(const Function&) = delete;
				Function& operator=(const Function&) = delete;
				
				Module& module_;
				llvm::Function& function_;
				
				llvm::IRBuilder<> entryBuilder_, builder_;
				bool useEntryBuilder_;
				
				const ArgInfo& argInfo_;
				TemplateBuilder* templateBuilder_;
				
				AlignMaskMap alignMaskMap_;
				LocalVarMap localVarMap_;
				MemberOffsetMap memberOffsetMap_;
				SizeOfMap sizeOfMap_;
				TemplateGeneratorMap templateGeneratorMap_;
				
				// A stack of unwind stacks.
				std::stack<UnwindStack> unwindStackStack_;
				
				llvm::DISubprogram debugInfo_;
				std::vector<llvm::Value*> argValues_;
				
				llvm::Value* exceptionInfo_;
				llvm::Value* returnValuePtr_;
				llvm::Value* templateArgs_;
				llvm::Value* unwindState_;
				
		};
		
		class SetUseEntryBuilder {
			public:
				inline SetUseEntryBuilder(Function& function)
					: function_(function) {
					previousValue_ = function_.setUseEntryBuilder(true);
				}
				
				inline ~SetUseEntryBuilder() {
					(void) function_.setUseEntryBuilder(previousValue_);
				}
				
			private:
				Function& function_;
				bool previousValue_;
				
		};
		
	}
	
}

#endif
