#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/Interpreter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/ModulePtr.hpp>

namespace locic {

	namespace CodeGen {
		
		class InterpreterImpl {
		public:
			InterpreterImpl(llvm::Module& argModule, llvm::ExecutionEngine* const argExecutionEngine)
			: module_(argModule), executionEngine_(argExecutionEngine) { }
			
			llvm::Module& module() {
				return module_;
			}
			
			const llvm::Module& module() const {
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
			llvm::Module& module_;
			llvm::ExecutionEngine* executionEngine_;
			
		};
		
		Interpreter::Interpreter(Context& /*context*/, ModulePtr module)
		: impl_(new InterpreterImpl(module->getLLVMModule(), nullptr)) {
			llvm::InitializeNativeTarget();
			llvm::InitializeNativeTargetAsmPrinter();
			
			llvm::TargetOptions targetOptions;
#if LOCIC_LLVM_VERSION < 304
			targetOptions.JITExceptionHandling = true;
#endif

#if LOCIC_LLVM_VERSION >= 306
			llvm::EngineBuilder engineBuilder(module->releaseLLVMModule());
#else
			llvm::EngineBuilder engineBuilder(module->releaseLLVMModule().release());
#endif
			
			engineBuilder.setEngineKind(llvm::EngineKind::JIT);
			engineBuilder.setTargetOptions(targetOptions);
			
#if LOCIC_LLVM_VERSION < 306
			engineBuilder.setUseMCJIT(true);
#endif
			
			std::string errorString;
			llvm::ExecutionEngine* const executionEngine = engineBuilder.setErrorStr(&errorString).create();
			if (executionEngine == nullptr) {
				throw std::runtime_error(std::string("Interpreter failed: Couldn't create execution engine with error: ") + errorString);
			}
			impl_->setExecutionEngine(executionEngine);
		}
		
		Interpreter::~Interpreter() { }
		
		void* Interpreter::getFunctionPointer(const std::string& functionName) {
			llvm::Function* const function = impl_->module().getFunction(functionName);
			if (function == nullptr) {
				throw std::runtime_error(std::string("Interpreter failed: No function '") + functionName + std::string("' exists in module."));
			}
			
			impl_->executionEngine()->finalizeObject();
			
			return impl_->executionEngine()->getPointerToFunction(function);
		}
		
		int Interpreter::runAsMain(const std::string& functionName, const std::vector<std::string>& args) {
			llvm::Function* const function = impl_->module().getFunction(functionName);
			if (function == nullptr) {
				throw std::runtime_error(std::string("Interpreter failed: No function '") + functionName + std::string("' exists in module."));
			}
			
			impl_->executionEngine()->finalizeObject();
			
			std::vector<llvm::GenericValue> gvArgs;
			llvm::GenericValue argc;
			argc.IntVal = llvm::APInt(32, args.size());
			gvArgs.push_back(argc);
			
			std::vector<const char*> argsArray;
			for (const auto& arg: args) {
				argsArray.push_back(arg.c_str());
			}
			gvArgs.push_back(llvm::PTOGV(argsArray.data()));
			
			return impl_->executionEngine()->runFunction(function, gvArgs).IntVal.getZExtValue();
		}
		
	}
	
}

