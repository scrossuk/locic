#ifndef LOCIC_DEBUG_STATEMENTINFO_HPP
#define LOCIC_DEBUG_STATEMENTINFO_HPP

#include <locic/Debug/SourceLocation.hpp>

namespace locic {

	namespace Debug {
		
		struct StatementInfo {
			SourceLocation location;
			
			inline StatementInfo()
				: location(SourceLocation::Null()) { }
		};
		
	}
	
}

#endif
