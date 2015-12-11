#include <memory>
#include <stdexcept>
#include <unordered_map>

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
			if (id.isCallable()) {
				// Currently callable primitives have multiple
				// type instances for the number of parameters.
				// TODO: Remove this when variadic templates
				//       are supported.
				constexpr size_t MAX_NUMBER_ARGUMENTS = 8;
				for (size_t i = 0; i <= MAX_NUMBER_ARGUMENTS; i++) {
					insertMapping(String(stringHost_, id.getName(i)),
					              id);
				}
			} else {
				insertMapping(String(stringHost_, id.toCString()),
				              id);
			}
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
		
		impl_->add(PrimitiveFunctionPtr);
		impl_->add(PrimitiveMethodFunctionPtr);
		impl_->add(PrimitiveTemplatedFunctionPtr);
		impl_->add(PrimitiveTemplatedMethodFunctionPtr);
		impl_->add(PrimitiveVarArgFunctionPtr);
		
		impl_->add(PrimitiveMethod);
		impl_->add(PrimitiveTemplatedMethod);
		impl_->add(PrimitiveInterfaceMethod);
		impl_->add(PrimitiveStaticInterfaceMethod);
		
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
		impl_->add(PrimitiveFinalLval);
		impl_->add(PrimitiveTypename);
		
		impl_->add(PrimitiveCount);
		impl_->add(PrimitiveCountIncl);
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
		
		throw std::logic_error(makeString("Unknown Primitive ID name '%s'.", name.c_str()));
	}
	
}
