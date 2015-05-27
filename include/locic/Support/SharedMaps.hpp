#ifndef LOCIC_SUPPORT_SHAREDMAPS_HPP
#define LOCIC_SUPPORT_SHAREDMAPS_HPP

#include <locic/Support/MethodIDMap.hpp>
#include <locic/Support/PrimitiveIDMap.hpp>
#include <locic/Support/StringHost.hpp>

namespace locic {
	
	class SharedMaps {
	public:
		SharedMaps()
		: stringHost_(),
		methodIDMap_(stringHost_),
		primitiveIDMap_(stringHost_) { }
		
		const StringHost& stringHost() const {
			return stringHost_;
		}
		
		const MethodIDMap& methodIDMap() const {
			return methodIDMap_;
		}
		
		const PrimitiveIDMap& primitiveIDMap() const {
			return primitiveIDMap_;
		}
		
	private:
		// Non-copyable.
		SharedMaps(const SharedMaps&) = delete;
		SharedMaps& operator=(const SharedMaps&) = delete;
		
		StringHost stringHost_;
		MethodIDMap methodIDMap_;
		PrimitiveIDMap primitiveIDMap_;
		
	};
	
}

#endif
