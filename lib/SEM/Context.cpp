#include <locic/StableSet.hpp>

#include <locic/SEM/Context.hpp>
#include <locic/SEM/Namespace.hpp>
#include <locic/SEM/Type.hpp>

namespace locic {

	namespace SEM {
	
		class ContextImpl {
		public:
			ContextImpl()
			: rootNamespace(new SEM::Namespace(Name::Absolute())) { }
			
			std::unique_ptr<Namespace> rootNamespace;
			mutable StableSet<Type> types;
		};
		
		// Allocate a large amount of space up-front for
		// possible types in the StableSet hash map.
		constexpr size_t TypesReserveCount = 5000;
		
		Context::Context()
		: impl_(new ContextImpl()) {
			impl_->types.reserve(TypesReserveCount);
		}
		
		Context::~Context() {
			printf("Context: Average = %f, Max = %f, Num values = %llu, Bucket count = %llu\n",
				impl_->types.load_factor(),
				impl_->types.max_load_factor(),
				(unsigned long long) impl_->types.unique_count(),
				(unsigned long long) impl_->types.bucket_count());
		}
		
		const Type* Context::getType(Type type) const {
			const auto result = impl_->types.insert(std::move(type));
			return &(*(result.first));
		}
		
		Namespace* Context::rootNamespace() {
			return impl_->rootNamespace.get();
		}
		
	}
	
}

