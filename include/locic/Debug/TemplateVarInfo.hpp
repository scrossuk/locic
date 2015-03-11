#ifndef LOCIC_DEBUG_TEMPLATEVARINFO_HPP
#define LOCIC_DEBUG_TEMPLATEVARINFO_HPP

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace Debug {
		
		struct TemplateVarInfo {
			SourceLocation declLocation;
			SourceLocation scopeLocation;
			
			TemplateVarInfo()
			: declLocation(SourceLocation::Null()), scopeLocation(SourceLocation::Null()) { }
		};
		
	}
	
}

#endif
