#include <memory>

#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/ModulePtr.hpp>

namespace locic {
	
	namespace CodeGen {
		
		ModulePtr::ModulePtr(std::unique_ptr<Module> module)
		: ptr_(std::move(module)) { }
		
		ModulePtr::~ModulePtr() { }
		
		ModulePtr::ModulePtr(ModulePtr&& other)
		: ptr_(std::move(other.ptr_)) { }
		
		ModulePtr& ModulePtr::operator=(ModulePtr&& other) {
			ptr_ = std::move(other.ptr_);
			return *this;
		}
		
		std::unique_ptr<Module> ModulePtr::release() {
			return std::move(ptr_);
		}
		
	}
	
}
