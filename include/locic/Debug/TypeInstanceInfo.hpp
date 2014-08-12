#ifndef LOCIC_DEBUG_TYPEINSTANCEINFO_HPP
#define LOCIC_DEBUG_TYPEINSTANCEINFO_HPP

#include <locic/Debug/SourceLocation.hpp>

namespace locic {

	namespace Debug {
		
		struct TypeInstanceInfo {
			SourceLocation location;
			
			inline TypeInstanceInfo()
				: location(SourceLocation::Null()) { }
			
			inline TypeInstanceInfo(SourceLocation pLocation)
				: location(pLocation) { }
		};
		
	}
	
}

#endif
