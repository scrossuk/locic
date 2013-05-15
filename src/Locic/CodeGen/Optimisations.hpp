#ifndef LOCIC_CODEGEN_OPTIMISATIONS_HPP
#define LOCIC_CODEGEN_OPTIMISATIONS_HPP

#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/Scalar.h>

#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
	
		class Optimisations {
			public:
				inline Optimisations(Module& module)
					: llvmModule_(module.getLLVMModule()),
					functionPassManager_(&llvmModule_),
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
				
				inline bool run() {
					bool changed = false;
					
					functionPassManager_.doInitialization();
					
					for (llvm::Module::iterator i = llvmModule_.begin(); i != llvmModule_.end(); ++i) {
						changed |= functionPassManager_.run(*i);
					}
					
					functionPassManager_.doFinalization();
					
					changed |= modulePassManager_.run(llvmModule_);
					
					return changed;
				}
				
			private:
				llvm::Module& llvmModule_;
				llvm::FunctionPassManager functionPassManager_;
				llvm::PassManager modulePassManager_;
				
		};
		
	}
	
}

#endif
