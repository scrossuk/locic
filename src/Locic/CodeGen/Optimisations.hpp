#ifndef LOCIC_CODEGEN_OPTIMISATIONS_HPP
#define LOCIC_CODEGEN_OPTIMISATIONS_HPP

#include <llvm/Transforms/IPO/PassManagerBuilder.h>

#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
	
		class Optimisations {
			public:
				inline Optimisations(const Module& module)
					: functionPassManager_(module_.getModule()),
					  modulePassManager_() { }
					  
				void addDefaultPasses(size_t optLevel, bool useInliner = true) {
					llvm::PassManagerBuilder passManagerBuilder;
					passManagerBuilder.OptLevel = optLevel;
					passManagerBuilder.Inliner =
						useInliner ?
						llvm::createFunctionInliningPass() :
						NULL;
						
					passManagerBuilder.populateFunctionPassManager(
						functionPassManager_);
					passManagerBuilder.populateModulePassManager(
						modulePassManager_);
				}
				
				inline bool run(Module& module) const {
					bool changed = false;
					
					llvm::Module& llvmModule = module.getLLVMModule();
					
					functionPassManager_.doInitialization();
					
					for (llvm::Module::iterator i = llvmModule.begin(); i != llvmModule.end(); ++i) {
						changed |= functionPassManager_.run(*i);
					}
					
					functionPassManager_.doFinalization();
					
					changed |= modulePassManager_.run(llvmModule);
					
					return changed;
				}
				
			private:
				mutable llvm::FunctionPassManager functionPassManager_;
				llvm::PassManager modulePassManager_;
				
		};
		
	}
	
}

#endif
