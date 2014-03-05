#ifndef LOCIC_DEBUG_VARINFO_HPP
#define LOCIC_DEBUG_VARINFO_HPP

#include <string>

#include <locic/Debug/SourceLocation.hpp>

namespace locic {

	namespace Debug {
		
		struct VarInfo {
			enum Kind {
				VAR_AUTO,
				VAR_ARG,
				VAR_MEMBER
			} kind;
			std::string name;
			SourceLocation declLocation;
			SourceLocation scopeLocation;
			
			inline VarInfo()
				: kind(VAR_AUTO), name(), declLocation(SourceLocation::Null()), scopeLocation(SourceLocation::Null()) { }
		};
		
	}
	
}

#endif
