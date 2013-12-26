#ifndef LOCIC_CODEGEN_INTERPRETER_HPP
#define LOCIC_CODEGEN_INTERPRETER_HPP

#include <string>
#include <vector>

namespace llvm {
	
	class ExecutionEngine;
	
}

namespace locic {

	namespace CodeGen {
	
		class Module;
		
		class Interpreter {
			public:
				Interpreter(Module& module);
				~Interpreter();
				
				void* getFunctionPointer(const std::string& functionName);
				
				int runAsMain(const std::string& functionName, const std::vector<std::string>& args);
				
			private:
				Module& module_;
				llvm::ExecutionEngine* executionEngine_;
				
		};
		
	}
	
}

#endif
