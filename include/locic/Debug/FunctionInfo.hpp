#ifndef LOCIC_DEBUG_FUNCTIONINFO_HPP
#define LOCIC_DEBUG_FUNCTIONINFO_HPP

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Name.hpp>

namespace locic {

	namespace Debug {
		
		struct FunctionInfo {
			bool isDefinition;
			Name name;
			SourceLocation declLocation;
			SourceLocation scopeLocation;
			
			inline FunctionInfo()
				: isDefinition(false), name(), declLocation(SourceLocation::Null()), scopeLocation(SourceLocation::Null()) { }
		};
		
	}
	
}

#endif
