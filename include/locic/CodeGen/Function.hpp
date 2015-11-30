#ifndef LOCIC_CODEGEN_FUNCTION_HPP
#define LOCIC_CODEGEN_FUNCTION_HPP

#include <map>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include <locic/CodeGen/UnwindAction.hpp>

#include <locic/Support/Map.hpp>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Builder.hpp>
#include <llvm-abi/FunctionEncoder.hpp>

namespace locic {
	
	class String;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace SEM {
		
		class Type;
		class TypeInstance;
		class Var;
		
	}
	
	namespace CodeGen {
		
		class ArgInfo;
		class Module;
		class TemplateBuilder;
		class TemplateInst;
		class UnwindAction;
		
		static const std::string NO_FUNCTION_NAME = "";
		
		llvm::Function* createLLVMFunction(Module& module, const ArgInfo& argInfo, llvm::GlobalValue::LinkageTypes linkage, const String& name);
		
		typedef std::unordered_map<const SEM::Type*, llvm::Value*> AlignMaskMap;
		typedef Map<const SEM::Var*, llvm::Value*> LocalVarMap;
		typedef std::pair<const SEM::Type*, size_t> OffsetPair;
		typedef std::map<OffsetPair, llvm::Value*> MemberOffsetMap;
		typedef std::unordered_map<const SEM::Type*, llvm::Value*> SizeOfMap;
		typedef std::map<TemplateInst, llvm::Value*> TemplateGeneratorMap;
		typedef std::vector<UnwindAction> UnwindStack;
		
		class Function: public llvm_abi::Builder {
			public:
				Function(Module& pModule, llvm::Function& function, const ArgInfo& argInfo, TemplateBuilder* templateBuilder = nullptr);
				
				/**
				 * \brief Generate return instruction
				 * 
				 * This creates a return instruction for the value, passing
				 * it out of the function by value (rather than setting a
				 * return value pointer).
				 * 
				 * This method handles encoding the value according to the ABI.
				 */
				llvm::Instruction* returnValue(llvm::Value* value);
				
				/**
				 * \brief Set return value for later return
				 * 
				 * This creates an instruction that stores the value into
				 * allocated stack memory. A subsequent call to
				 * getRawReturnValue() can then retrieve this.
				 * 
				 * This functionality is useful if the return value must
				 * be stored before executing some number of unwind
				 * actions (e.g. calling destructors) and then eventually
				 * loading and returning this value.
				 * 
				 * This method handles encoding the value according to the ABI.
				 */
				void setReturnValue(llvm::Value* value);
				
				/**
				 * \brief Retrieve previously stored return value
				 * 
				 * Retrieves the previously stored return value from
				 * setReturnValue().
				 */
				llvm::Value* getRawReturnValue();
				
				llvm::Function& getLLVMFunction();
				
				const llvm::Function& getLLVMFunction() const;
				
				llvm::Function* getLLVMFunctionPtr();
				
				Module& module();
				
				const Module& module() const;
				
				const ArgInfo& getArgInfo() const;
				
				llvm::Value* getRawArg(size_t index) const;
				
				llvm::Value* getArg(size_t index) const;
				
				std::vector<llvm::Value*> getArgList() const;
				
				llvm::Value* getNestArgument() const;
				
				llvm::Value* getTemplateGenerator() const;
				
				llvm::Value* getTemplateArgs();
				
				llvm::Value* getReturnVar() const;
				
				llvm::Value* getReturnVarOrNull() const;
				
				llvm::Value* getContextValue() const;
				
				TemplateBuilder& templateBuilder();
				
				llvm::BasicBlock* createBasicBlock(const std::string& name);
				
				// Returns an 'entry' builder for creating instructions
				// in the first ('entry') basic block.
				llvm::IRBuilder<>& getEntryBuilder();
				const llvm::IRBuilder<>& getEntryBuilder() const;
				
				llvm::IRBuilder<>& getBuilder();
				const llvm::IRBuilder<>& getBuilder() const;
				
				bool setUseEntryBuilder(bool useEntryBuilder);
				
				llvm::BasicBlock* getSelectedBasicBlock() const;
				
				void selectBasicBlock(llvm::BasicBlock* basicBlock);
				
				bool lastInstructionTerminates() const;
				
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
				
				void attachDebugInfo(DISubprogram subprogram);
				
				DISubprogram debugInfo() const;
				
				void setDebugPosition(const Debug::SourcePosition& position);
				
				llvm::DebugLoc getDebugLoc() const;
				
				void setPersonalityFunction(llvm::Constant* personalityFunction);
				
				llvm::Constant* personalityFunction() const;
				
			private:
				// Non-copyable.
				Function(const Function&) = delete;
				Function& operator=(const Function&) = delete;
				
				Module& module_;
				llvm::Function& function_;
				
				llvm::IRBuilder<> entryBuilder_, builder_;
				bool createdEntryBlock_;
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
				
				DISubprogram debugInfo_;
				
#if LOCIC_LLVM_VERSION < 307
				llvm::Constant* personalityFunction_;
#endif
				
				std::unique_ptr<llvm_abi::FunctionEncoder> functionEncoder_;
				
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
