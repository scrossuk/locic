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
			
			static inline VarInfo Auto(const std::string& name, const SourceLocation& declLocation, const SourceLocation& scopeLocation) {
				return VarInfo(VAR_AUTO, name, declLocation, scopeLocation);
			}
			
			static inline VarInfo Arg(const std::string& name, const SourceLocation& declLocation, const SourceLocation& scopeLocation) {
				return VarInfo(VAR_ARG, name, declLocation, scopeLocation);
			}
			
			static inline VarInfo Member(const std::string& name, const SourceLocation& declLocation, const SourceLocation& scopeLocation) {
				return VarInfo(VAR_MEMBER, name, declLocation, scopeLocation);
			}
			
			inline VarInfo(Kind pKind, const std::string& pName, const SourceLocation& pDeclLocation, const SourceLocation& pScopeLocation)
				: kind(pKind), name(pName), declLocation(pDeclLocation), scopeLocation(pScopeLocation) { }
		};
		
	}
	
}

#endif
