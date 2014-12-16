#include <set>

#include <locic/SEM/Context.hpp>
#include <locic/SEM/Namespace.hpp>
#include <locic/SEM/Type.hpp>

namespace locic {

	namespace SEM {
	
		Context::Context()
			: rootNamespace_(new SEM::Namespace(Name::Absolute())) { }
		
		const Type* Context::getType(Type type) const {
			const auto result = types_.insert(std::move(type));
			return &(*(result.first));
		}
		
		Namespace* Context::rootNamespace() {
			return rootNamespace_.get();
		}
		
	}
	
}

