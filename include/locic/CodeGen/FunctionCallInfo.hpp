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
		
		bool isTrivialFunction(Module& module, SEM::Value* value);
		
		llvm::Value* genTrivialFunctionCall(Function& function, SEM::Value* value, bool passContextByRef, llvm::ArrayRef<llvm::Value*> args);
		
		FunctionCallInfo genFunctionCallInfo(Function& function, SEM::Value* value);
		
	}
	
}

#endif
