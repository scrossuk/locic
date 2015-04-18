#ifndef LOCIC_SUPPORT_METHODIDMAP_HPP
#define LOCIC_SUPPORT_METHODIDMAP_HPP

#include <memory>

#include <locic/Support/MethodID.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	class MethodIDMap {
	public:
		MethodIDMap(const StringHost& stringHost);
		~MethodIDMap();
		
		MethodID getMethodID(const String& name) const;
		
	private:
		// Non-copyable.
		MethodIDMap(const MethodIDMap&) = delete;
		MethodIDMap& operator=(const MethodIDMap&) = delete;
		
		std::unique_ptr<class MethodIDMapImpl> impl_;
		
	};
	
}

#endif
