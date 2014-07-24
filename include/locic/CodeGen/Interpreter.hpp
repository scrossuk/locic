#ifndef LOCIC_CODEGEN_INTERPRETER_HPP
#define LOCIC_CODEGEN_INTERPRETER_HPP

#include <string>
#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>

namespace locic {

	namespace CodeGen {
	
		class Module;
		
		class Interpreter {
			public:
				Interpreter(llvm::Module* module);
				~Interpreter();
				
				void* getFunctionPointer(const std::string& functionName);
				
				int runAsMain(const std::string& functionName, const std::vector<std::string>& args);
				
			private:
				llvm::Module* module_;
				llvm::ExecutionEngine* executionEngine_;
				
		};
		
	}
	
}

#endif
