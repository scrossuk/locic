#include <string>

#include <locic/CodeGen/Linker.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Module* loadBitcodeFile(const std::string& fileName, llvm::LLVMContext& context) {
			#ifdef LLVM_3_3
			llvm::sys::Path fileNamePath;
			if (!fileNamePath.set(fileName)) {
				throw Exception(STR("Invalid file name '%s'.", fileName.c_str()));
			}
			
			llvm::SMDiagnostic error;
			return llvm::ParseIRFile(fileNamePath.str(), error, context);
			#else
			llvm::SMDiagnostic error;
			return llvm::ParseIRFile(fileName, error, context);
			#endif
		}
		
		class LinkerImpl {
		public:
			LinkerImpl(Module& argModule)
			: module_(argModule) { }
			
			Module& linkedModule() {
				return module_;
			}
			
			const Module& linkedModule() const {
				return module_;
			}
			
		private:
			Module& module_;
			
		};
		
		Linker::Linker(Context& /*context*/, Module& argModule)
		: impl_(new LinkerImpl(argModule)) { }
		
		Linker::~Linker() { }
		
		void Linker::loadModule(const std::string& fileName) {
			const auto loadedModule = loadBitcodeFile(fileName, impl_->linkedModule().getLLVMContext());
			if (loadedModule == nullptr) {
				throw std::runtime_error(makeString("Failed to load module '%s'.", fileName.c_str()));
			}
			
			std::string errorMessage;
			const bool linkFailed = llvm::Linker::LinkModules(impl_->linkedModule().getLLVMModulePtr(), loadedModule, llvm::Linker::DestroySource, &errorMessage);
			if (linkFailed) {
				throw std::runtime_error(makeString("Couldn't link with module '%s'; error given was '%s'.",
					fileName.c_str(), errorMessage.c_str()));
			}
		}
		
		Module& Linker::module() {
			return impl_->linkedModule();
		}
		
		const Module& Linker::module() const {
			return impl_->linkedModule();
		}
		
	}
	
}

