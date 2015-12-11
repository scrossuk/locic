#include <memory>
#include <stdexcept>
#include <unordered_map>

#include <locic/Support/MethodID.hpp>
#include <locic/Support/MethodIDMap.hpp>
#include <locic/Support/Optional.hpp>
#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/PrimitiveIDMap.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/StringHost.hpp>

namespace locic {
	
	class MethodIDMapImpl {
	public:
		MethodIDMapImpl(const StringHost& stringHost,
		                const PrimitiveIDMap& primitiveIDMap)
		: stringHost_(stringHost),
		  primitiveIDMap_(primitiveIDMap) { }
		
		inline void add(const MethodID id) {
			map_.insert(std::make_pair(String(stringHost_, id.toCString()), id));
		}
		
		const StringHost& stringHost_;
		const PrimitiveIDMap& primitiveIDMap_;
		std::unordered_map<String, MethodID> map_;
		
	};
	
	MethodIDMap::MethodIDMap(const StringHost& stringHost,
	                         const PrimitiveIDMap& primitiveIDMap)
	: impl_(new MethodIDMapImpl(stringHost,
	                            primitiveIDMap)) {
		impl_->add(METHOD_CREATE);
		impl_->add(METHOD_DEAD);
		impl_->add(METHOD_NULL);
		impl_->add(METHOD_ZERO);
		impl_->add(METHOD_UNIT);
		impl_->add(METHOD_LEADINGONES);
		impl_->add(METHOD_TRAILINGONES);
		impl_->add(METHOD_LEADINGZEROES);
		impl_->add(METHOD_TRAILINGZEROES);
		impl_->add(METHOD_ALIGNMASK);
		impl_->add(METHOD_SIZEOF);
		impl_->add(METHOD_UNINITIALIZED);
		
		impl_->add(METHOD_DESTROY);
		impl_->add(METHOD_IMPLICITCAST);
		impl_->add(METHOD_CAST);
		impl_->add(METHOD_IMPLICITCOPY);
		impl_->add(METHOD_COPY);
		impl_->add(METHOD_PLUS);
		impl_->add(METHOD_MINUS);
		impl_->add(METHOD_NOT);
		impl_->add(METHOD_FRONT);
		impl_->add(METHOD_SKIPFRONT);
		impl_->add(METHOD_BACK);
		impl_->add(METHOD_SKIPBACK);
		impl_->add(METHOD_EMPTY);
		impl_->add(METHOD_ISZERO);
		impl_->add(METHOD_ISPOSITIVE);
		impl_->add(METHOD_ISNEGATIVE);
		impl_->add(METHOD_ABS);
		impl_->add(METHOD_ADDRESS);
		impl_->add(METHOD_DEREF);
		impl_->add(METHOD_DISSOLVE);
		impl_->add(METHOD_MOVE);
		impl_->add(METHOD_SIGNEDVALUE);
		impl_->add(METHOD_UNSIGNEDVALUE);
		impl_->add(METHOD_COUNTLEADINGZEROES);
		impl_->add(METHOD_COUNTLEADINGONES);
		impl_->add(METHOD_COUNTTRAILINGZEROES);
		impl_->add(METHOD_COUNTTRAILINGONES);
		impl_->add(METHOD_SQRT);
		impl_->add(METHOD_INCREMENT);
		impl_->add(METHOD_DECREMENT);
		impl_->add(METHOD_SETDEAD);
		impl_->add(METHOD_ISLIVE);
		impl_->add(METHOD_SETINVALID);
		impl_->add(METHOD_ISVALID);
		impl_->add(METHOD_ISEQUAL);
		impl_->add(METHOD_ISNOTEQUAL);
		impl_->add(METHOD_ISLESSTHAN);
		impl_->add(METHOD_ISLESSTHANOREQUAL);
		impl_->add(METHOD_ISGREATERTHAN);
		impl_->add(METHOD_ISGREATERTHANOREQUAL);
		
		impl_->add(METHOD_ADD);
		impl_->add(METHOD_SUBTRACT);
		impl_->add(METHOD_MULTIPLY);
		impl_->add(METHOD_DIVIDE);
		impl_->add(METHOD_MODULO);
		impl_->add(METHOD_COMPARE);
		impl_->add(METHOD_ASSIGN);
		impl_->add(METHOD_INDEX);
		impl_->add(METHOD_EQUAL);
		impl_->add(METHOD_NOTEQUAL);
		impl_->add(METHOD_LESSTHAN);
		impl_->add(METHOD_LESSTHANOREQUAL);
		impl_->add(METHOD_GREATERTHAN);
		impl_->add(METHOD_GREATERTHANOREQUAL);
		impl_->add(METHOD_BITWISEAND);
		impl_->add(METHOD_BITWISEOR);
		impl_->add(METHOD_LEFTSHIFT);
		impl_->add(METHOD_RIGHTSHIFT);
		
		impl_->add(METHOD_CALL);
		impl_->add(METHOD_MOVETO);
		impl_->add(METHOD_INRANGE);
		impl_->add(METHOD_SETVALUE);
		impl_->add(METHOD_EXTRACTVALUE);
		impl_->add(METHOD_DESTROYVALUE);
	}
	
	MethodIDMap::~MethodIDMap() { }
	
	Optional<MethodID> MethodIDMap::tryGetMethodID(const String& name) const {
		const auto iterator = impl_->map_.find(name);
		if (iterator != impl_->map_.end()) {
			return make_optional(iterator->second);
		}
		
		if (name.starts_with("implicitcast")) {
			assert(name != "implicitcast");
			constexpr size_t IMPLICITCAST_LENGTH = 12;
			const auto primitiveName = name.substr(IMPLICITCAST_LENGTH);
			return make_optional(MethodID(METHOD_IMPLICITCASTFROM,
			                              impl_->primitiveIDMap_.getPrimitiveID(primitiveName)));
		} else if (name.starts_with("cast")) {
			assert(name != "cast");
			constexpr size_t CAST_LENGTH = 4;
			const auto primitiveName = name.substr(CAST_LENGTH);
			return make_optional(MethodID(METHOD_CASTFROM,
			                              impl_->primitiveIDMap_.getPrimitiveID(primitiveName)));
		}
		
		return None;
	}
	
	MethodID MethodIDMap::getMethodID(const String& name) const {
		const auto methodID = tryGetMethodID(name);
		if (methodID) {
			return *methodID;
		}
		
		printf("%s\n", name.c_str());
		
		throw std::logic_error("Unknown Method ID name.");
	}
	
}
