#ifndef LOCIC_CODEGEN_FUNCTIONCALLINFO_HPP
#define LOCIC_CODEGEN_FUNCTIONCALLINFO_HPP

#include <locic/SEM.hpp>

#include <locic/CodeGen/ArgPair.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>

namespace locic {
	
	namespace CodeGen {
		
		struct FunctionCallInfo {
			llvm::Value* functionPtr;
			llvm::Value* templateGenerator;
			llvm::Value* contextPointer;
			
			FunctionCallInfo()
				: functionPtr(nullptr),
				templateGenerator(nullptr),
				contextPointer(nullptr) { }
		};
		
		llvm::Function* genFunctionRef(Module& module, const SEM::Type* parentType, SEM::Function* function);
		
		bool isTrivialFunction(Module& module, const SEM::Value& value);
		
		llvm::Value* genTrivialFunctionCall(Function& function, const SEM::Value& value, llvm::ArrayRef<SEM::Value> args, ArgPair contextValue = ArgPair(nullptr, 0));
		
		FunctionCallInfo genFunctionCallInfo(Function& function, const SEM::Value& value);
		
	}
	
}

#endif
