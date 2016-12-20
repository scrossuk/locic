#include <locic/AST/Context.hpp>
#include <locic/AST/FunctionType.hpp>
#include <locic/AST/Namespace.hpp>
#include <locic/AST/Type.hpp>

#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/StableSet.hpp>

namespace locic {
	
	namespace AST {
		
		class ContextImpl {
		public:
			ContextImpl() {
				for (size_t i = 0; i < PRIMITIVE_COUNT; i++) {
					primitiveTypes[i] = nullptr;
				}
			}
			
			mutable StableSet<FunctionTypeData> functionTypes;
			mutable StableSet<Type> types;
			mutable const TypeInstance* primitiveTypes[PRIMITIVE_COUNT];
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
		
		void Context::setPrimitive(const PrimitiveID primitiveID,
		                           const TypeInstance& typeInstance) {
			assert(impl_->primitiveTypes[primitiveID] == nullptr);
			impl_->primitiveTypes[primitiveID] = &typeInstance;
		}
		
		const TypeInstance&
		Context::getPrimitive(const PrimitiveID primitiveID) const {
			assert(impl_->primitiveTypes[primitiveID] != nullptr);
			return *(impl_->primitiveTypes[primitiveID]);
			
		}
		
	}
	
}

