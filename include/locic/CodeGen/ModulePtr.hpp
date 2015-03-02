#ifndef LOCIC_CODEGEN_MODULEPTR_HPP
#define LOCIC_CODEGEN_MODULEPTR_HPP

#include <memory>

namespace locic {
	
	namespace CodeGen {
		
		class Module;
		
		/**
		 * \brief Module Unique Pointer
		 * 
		 * This type exists because std::unique_ptr<Module> cannot
		 * destroy the module it points to when the Module type
		 * is incomplete (which happens when other parts of the
		 * compiler tools use the CodeGen APIs).
		 */
		class ModulePtr {
			public:
				ModulePtr(std::unique_ptr<Module> module);
				~ModulePtr();
				
				ModulePtr(ModulePtr&&);
				ModulePtr& operator=(ModulePtr&&);
				
				Module& operator*() {
					return *ptr_;
				}
				
				const Module& operator*() const {
					return *ptr_;
				}
				
				Module* operator->() {
					return ptr_.get();
				}
				
				const Module* operator->() const {
					return ptr_.get();
				}
				
				std::unique_ptr<Module> release();
				
			private:
				std::unique_ptr<Module> ptr_;
				
		};
		
	}
	
}

#endif
