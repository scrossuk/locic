#include <string>

#include <locic/CodeGen/Linker.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/ModulePtr.hpp>

namespace locic {

	namespace CodeGen {
		
		std::unique_ptr<llvm::Module> loadBitcodeFile(const std::string& fileName, llvm::LLVMContext& context) {
			llvm::SMDiagnostic error;
			return llvm::parseIRFile(fileName, error, context);
		}
		
		class LinkerImpl {
		public:
			LinkerImpl(std::unique_ptr<Module> argModule)
			: module_(std::move(argModule)) { }
			
			Module& linkedModule() {
				return *module_;
			}
			
			const Module& linkedModule() const {
				return *module_;
			}
			
			std::unique_ptr<Module> releaseModule() {
				auto releasedValue = std::move(module_);
				module_ = std::unique_ptr<Module>();
				return releasedValue;
			}
			
		private:
			std::unique_ptr<Module> module_;
			
		};
		
		Linker::Linker(Context& /*context*/, ModulePtr argModule)
		: impl_(new LinkerImpl(argModule.release())) { }
		
		Linker::~Linker() { }
		
		void Linker::loadModule(const std::string& fileName) {
			auto loadedModule = loadBitcodeFile(fileName, impl_->linkedModule().getLLVMContext());
			if (loadedModule == nullptr) {
				throw std::runtime_error(makeString("Failed to load module '%s'.", fileName.c_str()));
			}
			
			std::string errorMessage;
			const bool linkFailed = llvm::Linker::linkModules(impl_->linkedModule().getLLVMModule(), std::move(loadedModule));
			if (linkFailed) {
				throw std::runtime_error(makeString("Couldn't link with module '%s'; error given was '%s'.",
					fileName.c_str(), errorMessage.c_str()));
			}
			
			impl_->linkedModule().verify();
		}
		
		Module& Linker::module() {
			return impl_->linkedModule();
		}
		
		const Module& Linker::module() const {
			return impl_->linkedModule();
		}
		
		ModulePtr Linker::releaseModule() {
			return impl_->releaseModule();
		}
		
	}
	
}

