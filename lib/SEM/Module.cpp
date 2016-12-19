#include <string>

#include <locic/AST/Namespace.hpp>

#include <locic/AST/Context.hpp>
#include <locic/SEM/Module.hpp>

#include <locic/Support/MakeString.hpp>

namespace locic {
	
	namespace SEM {
		
		Module::Module(AST::Context& pContext)
		: context_(pContext), rootNamespace_(new AST::Namespace()) { }
		
		Module::~Module() { }
		
		AST::Context& Module::context() {
			return context_;
		}
		
		const AST::Context& Module::context() const {
			return context_;
		}
		
		AST::Namespace& Module::rootNamespace() {
			return *rootNamespace_;
		}
		
		const AST::Namespace& Module::rootNamespace() const {
			return *rootNamespace_;
		}
		
		std::string Module::toString() const {
			return makeString("Module(rootNamespace: %s)",
			                  rootNamespace_->toString().c_str());
		}
		
	}
	
}
