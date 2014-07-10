#include <set>

#include <locic/SEM/Context.hpp>
#include <locic/SEM/Namespace.hpp>
#include <locic/SEM/Type.hpp>

namespace locic {

	namespace SEM {
	
		Context::Context()
			: rootNamespace_(new SEM::Namespace(Name::Absolute())) { }
		
		Type* Context::getType(Type type) {
			auto result = types_.insert(type);
			// Not sure why the const cast is needed here...
			return const_cast<Type*>(&(*(result.first)));
		}
		
		SEM::Namespace* Context::rootNamespace() {
			return rootNamespace_.get();
		}
		
	}
	
}

