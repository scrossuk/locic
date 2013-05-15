#ifndef LOCIC_CODEGEN_TYPEMAPPING_HPP
#define LOCIC_CODEGEN_TYPEMAPPING_HPP

#include <string>
#include <vector>

#include <Locic/Name.hpp>
#include <Locic/SEM.hpp>

namespace Locic {

	namespace CodeGen {
	
		/*struct Context {
			std::string name;
			TargetInfo targetInfo_;
			llvm::Module* module;
			llvm::IRBuilder<> builder;
			llvm::FunctionType* currentFunctionType_;
			llvm::Function* currentFunction_;
			llvm::BasicBlock* currentBasicBlock_;
			
			Map<SEM::TypeInstance*, llvm::StructType*> typeInstances_;
			
			Map<SEM::Function*, llvm::Function*> functions_;
			Map<SEM::Var*, size_t> memberVarOffsets_;
			Map<SEM::Var*, llvm::Value*> localVariables_, paramVariables_;
			llvm::Value* returnVar_;
			llvm::Value* thisPointer_;
			llvm::StructType* typenameType_;
		};*/
		
		class TypeMapping {
			public:
				TypeMapping();
				~TypeMapping();
				
				void add(SEM::TypeInstance* fromType, llvm::StructType* toType);
				
				llvm::StructType* get(SEM::TypeInstance* type);
				
			private:
				Map<SEM::TypeInstance*, llvm::StructType*> map_;
				
		};
		
	}
	
}

#endif
