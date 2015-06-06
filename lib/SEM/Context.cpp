#include <locic/Support/StableSet.hpp>

#include <locic/SEM/Context.hpp>
#include <locic/SEM/FunctionType.hpp>
#include <locic/SEM/Namespace.hpp>
#include <locic/SEM/Type.hpp>

namespace locic {

	namespace SEM {
	
		class ContextImpl {
		public:
			ContextImpl()
			: rootNamespace(new SEM::Namespace(Name::Absolute())) { }
			
			std::unique_ptr<Namespace> rootNamespace;
			mutable StableSet<FunctionTypeData> functionTypes;
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
		}
		
		FunctionType Context::getFunctionType(FunctionTypeData functionType) const {
			const auto result = impl_->functionTypes.insert(std::move(functionType));
			return FunctionType(*(result.first));
		}
		
		const Type* Context::getType(Type&& type) const {
			const auto result = impl_->types.insert(std::move(type));
			return &(*(result.first));
		}
		
		Namespace* Context::rootNamespace() {
			return impl_->rootNamespace.get();
		}
		
	}
	
}

