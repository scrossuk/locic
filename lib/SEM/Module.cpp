#include <string>

#include <locic/SEM/Context.hpp>
#include <locic/SEM/Module.hpp>
#include <locic/SEM/Namespace.hpp>

#include <locic/Support/MakeString.hpp>

namespace locic {
	
	namespace SEM {
		
		Module::Module(Context& context)
		: context_(context), rootNamespace_(new Namespace()) { }
		
		Module::~Module() { }
		
		Context& Module::context() {
			return context_;
		}
		
		const Context& Module::context() const {
			return context_;
		}
		
		Namespace& Module::rootNamespace() {
			return *rootNamespace_;
		}
		
		const Namespace& Module::rootNamespace() const {
			return *rootNamespace_;
		}
		
		std::string Module::toString() const {
			return makeString("Module(rootNamespace: %s)",
			                  rootNamespace_->toString().c_str());
		}
		
	}
	
}
