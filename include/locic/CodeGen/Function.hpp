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
	
	namespace AST {
		
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
		
		typedef std::unordered_map<const AST::Type*, llvm::Value*> AlignMaskMap;
		typedef Map<const AST::Var*, llvm::Value*> LocalVarMap;
		typedef std::pair<const AST::Type*, size_t> OffsetPair;
		typedef std::map<OffsetPair, llvm::Value*> MemberOffsetMap;
		typedef std::unordered_map<const AST::Type*, llvm::Value*> SizeOfMap;
		typedef std::map<TemplateInst, llvm::Value*> TemplateGeneratorMap;
		typedef std::vector<UnwindAction> UnwindStack;
		
		class Function: public llvm_abi::Builder {
		public:
			Function(Module& pModule, llvm::Function& function, const ArgInfo& argInfo, TemplateBuilder* templateBuilder = nullptr);
			
			void dump() const;
			
			/**
			 * \brief Get unwind return pointer.
			 * 
			 * This returns a previously set pointer given
			 * to setUnwindReturnPtr().
			 */
			llvm::Value* getUnwindReturnPtr();
			
			/**
			 * \brief Get unwind return pointer or NULL.
			 * 
			 * This returns a previously set pointer given
			 * to setUnwindReturnPtr(), or NULL if none has
			 * been set.
			 */
			llvm::Value* getUnwindReturnPtrOrNull();
			
			/**
			 * \brief Set unwind return pointer.
			 */
			void setUnwindReturnPtr(llvm::Value* ptr);
			
			llvm::Value* getVarAddress(const AST::Var& var);
			
			void setVarAddress(const AST::Var& var,
			                   llvm::Value* varPtr);
			
			llvm::Function& getLLVMFunction();
			
			const llvm::Function& getLLVMFunction() const;
			
			llvm::Function* getLLVMFunctionPtr();
			
			Module& module();
			
			const Module& module() const;
			
			const ArgInfo& getArgInfo() const;
			
			llvm_abi::FunctionEncoder& abiEncoder();
			
			llvm::Value* getRawArg(size_t index) const;
			
			llvm::Value* getArg(size_t index) const;
			
			std::vector<llvm::Value*> getArgList() const;
			
			llvm::Value* getNestArgument() const;
			
			llvm::Value* getTemplateGenerator() const;
			
			llvm::Value* getTemplateGeneratorOrNull() const;
			
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
			
			std::unique_ptr<llvm_abi::FunctionEncoder> functionEncoder_;
			
			llvm::Value* exceptionInfo_;
			llvm::Value* templateArgs_;
			llvm::Value* unwindReturnPtr_;
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
