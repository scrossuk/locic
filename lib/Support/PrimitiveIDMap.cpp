#include <memory>
#include <unordered_map>

#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/PrimitiveIDMap.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/StringHost.hpp>

namespace locic {
	
	class PrimitiveIDMapImpl {
	public:
		PrimitiveIDMapImpl(const StringHost& stringHost)
		: stringHost_(stringHost) { }
		
		void add(const PrimitiveID id) {
			insertMapping(String(stringHost_, id.toCString()), id);
		}
		
		void insertMapping(const String name, const PrimitiveID id) {
			map_.insert(std::make_pair(name, id));
			
			// Also insert canonicalized name since PrimitiveIDs
			// can appear in names of cast-from methods (e.g.
			// 'cast_uint8_t').
			map_.insert(std::make_pair(CanonicalizeMethodName(name),
			                           id));
		}
		
		const StringHost& stringHost_;
		std::unordered_map<String, PrimitiveID> map_;
		
	};
	
	PrimitiveIDMap::PrimitiveIDMap(const StringHost& stringHost)
	: impl_(new PrimitiveIDMapImpl(stringHost)) {
		impl_->add(PrimitiveVoid);
		impl_->add(PrimitiveNull);
		impl_->add(PrimitiveBool);
		impl_->add(PrimitiveCompareResult);
		impl_->add(PrimitiveUnichar);
		
#define ADD_CALLABLE_ID(id) \
	impl_->add(id ## 0); \
	impl_->add(id ## 1); \
	impl_->add(id ## 2); \
	impl_->add(id ## 3); \
	impl_->add(id ## 4); \
	impl_->add(id ## 5); \
	impl_->add(id ## 6); \
	impl_->add(id ## 7); \
	impl_->add(id ## 8)
		
		ADD_CALLABLE_ID(PrimitiveFunctionPtr);
		ADD_CALLABLE_ID(PrimitiveMethodFunctionPtr);
		ADD_CALLABLE_ID(PrimitiveTemplatedFunctionPtr);
		ADD_CALLABLE_ID(PrimitiveTemplatedMethodFunctionPtr);
		ADD_CALLABLE_ID(PrimitiveVarArgFunctionPtr);
		
		ADD_CALLABLE_ID(PrimitiveMethod);
		ADD_CALLABLE_ID(PrimitiveTemplatedMethod);
		ADD_CALLABLE_ID(PrimitiveInterfaceMethod);
		ADD_CALLABLE_ID(PrimitiveStaticInterfaceMethod);
		
		impl_->add(PrimitiveInt8);
		impl_->add(PrimitiveUInt8);
		impl_->add(PrimitiveInt16);
		impl_->add(PrimitiveUInt16);
		impl_->add(PrimitiveInt32);
		impl_->add(PrimitiveUInt32);
		impl_->add(PrimitiveInt64);
		impl_->add(PrimitiveUInt64);
		
		impl_->add(PrimitiveByte);
		impl_->add(PrimitiveUByte);
		impl_->add(PrimitiveShort);
		impl_->add(PrimitiveUShort);
		impl_->add(PrimitiveInt);
		impl_->add(PrimitiveUInt);
		impl_->add(PrimitiveLong);
		impl_->add(PrimitiveULong);
		impl_->add(PrimitiveLongLong);
		impl_->add(PrimitiveULongLong);
		
		impl_->add(PrimitiveSize);
		impl_->add(PrimitiveSSize);
		impl_->add(PrimitivePtrDiff);
		
		impl_->add(PrimitiveFloat);
		impl_->add(PrimitiveDouble);
		impl_->add(PrimitiveLongDouble);
		impl_->add(PrimitiveRef);
		impl_->add(PrimitivePtr);
		impl_->add(PrimitivePtrLval);
		impl_->add(PrimitiveValueLval);
		impl_->add(PrimitiveAbstractTypename);
		impl_->add(PrimitiveTypename);
		
		impl_->add(PrimitiveRange);
		impl_->add(PrimitiveRangeIncl);
		impl_->add(PrimitiveReverseRange);
		impl_->add(PrimitiveReverseRangeIncl);
		
		impl_->add(PrimitiveStaticArray);
	}
	
	PrimitiveIDMap::~PrimitiveIDMap() { }
	
	PrimitiveID PrimitiveIDMap::getPrimitiveID(const String& name) const {
		const auto iterator = impl_->map_.find(name);
		if (iterator != impl_->map_.end()) {
			return iterator->second;
		}
		
		locic_unreachable("Unknown Primitive ID name.");
	}
	
}
