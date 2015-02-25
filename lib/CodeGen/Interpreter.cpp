#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/Interpreter.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		class InterpreterImpl {
		public:
			InterpreterImpl(Module& argModule, llvm::ExecutionEngine* const argExecutionEngine)
			: module_(argModule), executionEngine_(argExecutionEngine) { }
			
			Module& module() {
				return module_;
			}
			
			const Module& module() const {
				return module_;
			}
			
			void setExecutionEngine(llvm::ExecutionEngine* const engine) {
				executionEngine_ = engine;
			}
			
			llvm::ExecutionEngine* executionEngine() {
				return executionEngine_;
			}
			
			const llvm::ExecutionEngine* executionEngine() const {
				return executionEngine_;
			}
			
		private:
			Module& module_;
			llvm::ExecutionEngine* executionEngine_;
			
		};
		
		Interpreter::Interpreter(Context& /*context*/, Module& module)
		: impl_(new InterpreterImpl(module, nullptr)) {
			llvm::InitializeNativeTarget();
			llvm::InitializeNativeTargetAsmPrinter();
			
			llvm::TargetOptions targetOptions;
#ifdef LLVM_3_3
			targetOptions.JITExceptionHandling = true;
#endif
			
			llvm::EngineBuilder engineBuilder(module.getLLVMModulePtr());
			
			engineBuilder.setEngineKind(llvm::EngineKind::JIT);
			engineBuilder.setTargetOptions(targetOptions);
			engineBuilder.setUseMCJIT(true);
			
			std::string errorString;
			llvm::ExecutionEngine* const executionEngine = engineBuilder.setErrorStr(&errorString).create();
			if (executionEngine == nullptr) {
				throw std::runtime_error(std::string("Interpreter failed: Couldn't create execution engine with error: ") + errorString);
			}
			impl_->setExecutionEngine(executionEngine);
		}
		
		Interpreter::~Interpreter() { }
		
		void* Interpreter::getFunctionPointer(const std::string& functionName) {
			llvm::Function* const function = impl_->module().getLLVMModule().getFunction(functionName);
			if (function == nullptr) {
				throw std::runtime_error(std::string("Interpreter failed: No function '") + functionName + std::string("' exists in module."));
			}
			
			impl_->executionEngine()->finalizeObject();
			
			return impl_->executionEngine()->getPointerToFunction(function);
		}
		
		int Interpreter::runAsMain(const std::string& functionName, const std::vector<std::string>& args) {
			llvm::Function* const function = impl_->module().getLLVMModule().getFunction(functionName);
			if (function == nullptr) {
				throw std::runtime_error(std::string("Interpreter failed: No function '") + functionName + std::string("' exists in module."));
			}
			
			impl_->executionEngine()->finalizeObject();
			
			return impl_->executionEngine()->runFunctionAsMain(function, args, NULL);
		}
		
	}
	
}

