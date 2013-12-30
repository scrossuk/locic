#ifndef LOCIC_SEM_NAMESPACE_HPP
#define LOCIC_SEM_NAMESPACE_HPP

#include <string>
#include <vector>

namespace locic {

	namespace SEM {
		
		class Function;
		class TypeInstance;
	
		class Namespace {
			public:
				Namespace(const std::string& n);
					
				const std::string& name() const;
				
				std::vector<Namespace *>& namespaces();
				const std::vector<Namespace *>& namespaces() const;
				
				std::vector<TypeInstance *>& typeInstances();
				const std::vector<TypeInstance *>& typeInstances() const;
				
				std::vector<Function *>& functions();
				const std::vector<Function *>& functions() const;
				
				std::string toString() const;
				
			private:
				std::string name_;
				std::vector<Namespace *> namespaces_;
				std::vector<TypeInstance *> typeInstances_;
				std::vector<Function *> functions_;
				
		};
		
	}
	
}

#endif
