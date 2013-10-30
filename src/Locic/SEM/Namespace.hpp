#ifndef LOCIC_SEM_NAMESPACE_HPP
#define LOCIC_SEM_NAMESPACE_HPP

#include <string>
#include <vector>

#include <Locic/SEM/Function.hpp>
#include <Locic/SEM/Object.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace Locic {

	namespace SEM {
	
		class Namespace: public Object {
			public:
				inline Namespace(const std::string& n)
					: name_(n) { }
				
				inline ObjectKind objectKind() const {
					return OBJECT_NAMESPACE;
				}
					
				inline const std::string& name() const {
					return name_;
				}
				
				inline std::vector<Namespace *>& namespaces() {
					return namespaces_;
				}
				
				inline const std::vector<Namespace *>& namespaces() const {
					return namespaces_;
				}
				
				inline std::vector<TypeInstance *>& typeInstances() {
					return typeInstances_;
				}
				
				inline const std::vector<TypeInstance *>& typeInstances() const {
					return typeInstances_;
				}
				
				inline std::vector<Function *>& functions() {
					return functions_;
				}
				
				inline const std::vector<Function *>& functions() const {
					return functions_;
				}
				
				inline std::string toString() const {
					return makeString("NameSpace(name: %s, "
						"namespaces: %s, typeInstances: %s, "
						"functions: %s)",
						name().c_str(),
						makeArrayString(namespaces_).c_str(),
						makeArrayString(typeInstances_).c_str(),
						makeArrayString(functions_).c_str()); 
				}
				
			private:
				std::string name_;
				std::vector<Namespace *> namespaces_;
				std::vector<TypeInstance *> typeInstances_;
				std::vector<Function *> functions_;
				
		};
		
	}
	
}

#endif
