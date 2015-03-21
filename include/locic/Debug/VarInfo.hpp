#ifndef LOCIC_DEBUG_VARINFO_HPP
#define LOCIC_DEBUG_VARINFO_HPP

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace Debug {
		
		struct VarInfo {
			enum Kind {
				VAR_LOCAL,
				VAR_ARGUMENT,
				VAR_MEMBER
			} kind;
			String name;
			SourceLocation declLocation;
			SourceLocation scopeLocation;
			
			VarInfo()
			: kind(VAR_LOCAL), declLocation(SourceLocation::Null()), scopeLocation(SourceLocation::Null()) { }
		};
		
	}
	
}

#endif
