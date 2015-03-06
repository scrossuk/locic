#include <string>

#include <locic/CodeGen/Linker.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/ModulePtr.hpp>

namespace locic {

	namespace CodeGen {
		
		std::unique_ptr<llvm::Module> loadBitcodeFile(const std::string& fileName, llvm::LLVMContext& context) {
#if defined(LLVM_3_3)
			llvm::sys::Path fileNamePath;
			if (!fileNamePath.set(fileName)) {
				throw Exception(STR("Invalid file name '%s'.", fileName.c_str()));
			}
			
			llvm::SMDiagnostic error;
			return std::unique_ptr<llvm::Module>(llvm::ParseIRFile(fileNamePath.str(), error, context));
#elif defined(LLVM_3_6)
			llvm::SMDiagnostic error;
			return llvm::parseIRFile(fileName, error, context);
#else
			llvm::SMDiagnostic error;
			return std::unique_ptr<llvm::Module>(llvm::ParseIRFile(fileName, error, context));
#endif
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
				return std::move(releasedValue);
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
#if defined(LLVM_3_6)
			const bool linkFailed = llvm::Linker::LinkModules(impl_->linkedModule().getLLVMModulePtr(), loadedModule.release(),
				[&](const llvm::DiagnosticInfo& info) {
					llvm::raw_string_ostream rawStream(errorMessage);
					llvm::DiagnosticPrinterRawOStream diagnosticStream(rawStream);
					info.print(diagnosticStream);
				}
			);
#else
			const bool linkFailed = llvm::Linker::LinkModules(impl_->linkedModule().getLLVMModulePtr(), loadedModule.release(), llvm::Linker::DestroySource, &errorMessage);
#endif
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

