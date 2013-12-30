#include <string>
#include <vector>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/Namespace.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {

	namespace SEM {
	
		Namespace::Namespace(const std::string& n)
			: name_(n) { }
			
		const std::string& Namespace::name() const {
			return name_;
		}
		
		std::vector<Namespace*>& Namespace::namespaces() {
			return namespaces_;
		}
		
		const std::vector<Namespace*>& Namespace::namespaces() const {
			return namespaces_;
		}
		
		std::vector<TypeInstance*>& Namespace::typeInstances() {
			return typeInstances_;
		}
		
		const std::vector<TypeInstance*>& Namespace::typeInstances() const {
			return typeInstances_;
		}
		
		std::vector<Function*>& Namespace::functions() {
			return functions_;
		}
		
		const std::vector<Function*>& Namespace::functions() const {
			return functions_;
		}
		
		std::string Namespace::toString() const {
			return makeString("NameSpace(name: %s, "
							  "namespaces: %s, typeInstances: %s, "
							  "functions: %s)",
							  name().c_str(),
							  makeArrayString(namespaces_).c_str(),
							  makeArrayString(typeInstances_).c_str(),
							  makeArrayString(functions_).c_str());
		}
		
	}
	
}

