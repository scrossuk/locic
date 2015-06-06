#ifndef LOCIC_DEBUG_VALUEINFO_HPP
#define LOCIC_DEBUG_VALUEINFO_HPP

#include <locic/Debug/SourceLocation.hpp>

namespace locic {

	namespace Debug {
		
		class ValueInfo {
		public:
			SourceLocation location;
			
			ValueInfo() : location(SourceLocation::Null()) { }
		};
		
	}
	
}

#endif
