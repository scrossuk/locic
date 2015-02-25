#ifndef LOCIC_CODEGEN_INTERPRETER_HPP
#define LOCIC_CODEGEN_INTERPRETER_HPP

#include <string>
#include <vector>

namespace locic {

	namespace CodeGen {
		
		class Context;
		class Module;
		
		class Interpreter {
			public:
				Interpreter(Context& context, Module& module);
				~Interpreter();
				
				void* getFunctionPointer(const std::string& functionName);
				
				int runAsMain(const std::string& functionName, const std::vector<std::string>& args);
				
			private:
				std::unique_ptr<class InterpreterImpl> impl_;
				
		};
		
	}
	
}

#endif
