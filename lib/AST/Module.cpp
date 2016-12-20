#include <string>

#include <locic/AST/Context.hpp>
#include <locic/AST/Module.hpp>
#include <locic/AST/Namespace.hpp>

#include <locic/Support/MakeString.hpp>

namespace locic {
	
	namespace AST {
		
		Module::Module(Context& pContext)
		: context_(pContext), rootNamespace_(new Namespace()) { }
		
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
