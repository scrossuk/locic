#ifndef LOCIC_SUPPORT_METHODIDMAP_HPP
#define LOCIC_SUPPORT_METHODIDMAP_HPP

#include <memory>

#include <locic/Support/Optional.hpp>

namespace locic {
	
	class MethodID;
	class PrimitiveIDMap;
	class String;
	class StringHost;
	
	class MethodIDMap {
	public:
		MethodIDMap(const StringHost& stringHost,
		            const PrimitiveIDMap& primitiveIDMap);
		~MethodIDMap();
		
		Optional<MethodID> tryGetMethodID(const String& name) const;
		
		MethodID getMethodID(const String& name) const;
		
	private:
		// Non-copyable.
		MethodIDMap(const MethodIDMap&) = delete;
		MethodIDMap& operator=(const MethodIDMap&) = delete;
		
		std::unique_ptr<class MethodIDMapImpl> impl_;
		
	};
	
}

#endif
