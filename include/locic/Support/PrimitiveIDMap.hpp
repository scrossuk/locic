#ifndef LOCIC_SUPPORT_PRIMITIVEIDMAP_HPP
#define LOCIC_SUPPORT_PRIMITIVEIDMAP_HPP

#include <memory>

#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	class PrimitiveIDMap {
	public:
		PrimitiveIDMap(const StringHost& stringHost);
		~PrimitiveIDMap();
		
		PrimitiveID getPrimitiveID(const String& name) const;
		
	private:
		// Non-copyable.
		PrimitiveIDMap(const PrimitiveIDMap&) = delete;
		PrimitiveIDMap& operator=(const PrimitiveIDMap&) = delete;
		
		std::unique_ptr<class PrimitiveIDMapImpl> impl_;
		
	};
	
}

#endif
