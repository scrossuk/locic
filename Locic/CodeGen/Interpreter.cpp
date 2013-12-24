#include <Locic/CodeGen/LLVMIncludes.hpp>

#include <stdexcept>
#include <string>
#include <vector>

#include <Locic/CodeGen/Interpreter.hpp>
#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
		
		Interpreter::Interpreter(Module& module)
			: module_(module),
			executionEngine_(NULL) {
				std::string errorString;
				executionEngine_ = llvm::EngineBuilder(module.getLLVMModulePtr()).setErrorStr(&errorString).create();
				if (executionEngine_ == NULL) {
					throw std::runtime_error(std::string("Interpreter failed: Couldn't create execution engine with error: ") + errorString);
				}
			}
		
		Interpreter::~Interpreter() { }
		
		void* Interpreter::getFunctionPointer(const std::string& functionName) {
			llvm::Function* function = module_.getLLVMModule().getFunction(functionName);
			if (function == NULL) {
				throw std::runtime_error(std::string("Interpreter failed: No function '") + functionName + std::string("' exists in module."));
			}
			return executionEngine_->getPointerToFunction(function);
		}
		
		int Interpreter::runAsMain(const std::string& functionName, const std::vector<std::string>& args) {
			llvm::Function* function = module_.getLLVMModule().getFunction(functionName);
			if (function == NULL) {
				throw std::runtime_error(std::string("Interpreter failed: No function '") + functionName + std::string("' exists in module."));
			}
			return executionEngine_->runFunctionAsMain(function, args, NULL);
		}
		
	}
	
}

