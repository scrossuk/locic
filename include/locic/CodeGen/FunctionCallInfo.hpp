#ifndef LOCIC_CODEGEN_FUNCTIONCALLINFO_HPP
#define LOCIC_CODEGEN_FUNCTIONCALLINFO_HPP

#include <locic/SEM.hpp>

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>

namespace locic {
	
	namespace CodeGen {
		
		struct FunctionCallInfo {
			llvm::Value* functionPtr;
			llvm::Value* templateGenerator;
			llvm::Value* contextPointer;
			
			inline FunctionCallInfo()
				: functionPtr(nullptr),
				templateGenerator(nullptr),
				contextPointer(nullptr) { }
		};
		
		typedef std::pair<llvm::Value*, bool> ArgPair;
		
		bool isTrivialFunction(Module& module, SEM::Value* value);
		
		llvm::Value* genTrivialFunctionCall(Function& function, SEM::Value* value, llvm::ArrayRef<SEM::Value*> args, ArgPair contextValue = ArgPair(nullptr, 0));
		
		FunctionCallInfo genFunctionCallInfo(Function& function, SEM::Value* value);
		
	}
	
}

#endif
