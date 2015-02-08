#ifndef LOCIC_DEBUG_VALUEINFO_HPP
#define LOCIC_DEBUG_VALUEINFO_HPP

#include <locic/Debug/SourceLocation.hpp>

namespace locic {

	namespace Debug {
		
		struct ValueInfo {
			SourceLocation location;
			
			ValueInfo() : location(SourceLocation::Null()) { }
		};
		
	}
	
}

#endif
