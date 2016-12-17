#ifndef LOCIC_DEBUG_FUNCTIONINFO_HPP
#define LOCIC_DEBUG_FUNCTIONINFO_HPP

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Support/Name.hpp>

namespace locic {

	namespace Debug {
		
		struct FunctionInfo {
			Name name;
			SourceLocation declLocation;
			SourceLocation scopeLocation;
			
			FunctionInfo()
			: name(), declLocation(SourceLocation::Null()), scopeLocation(SourceLocation::Null()) { }
			
			FunctionInfo(const FunctionInfo& other)
			: name(other.name.copy()),
			declLocation(other.declLocation),
			scopeLocation(other.scopeLocation) { }
			
			FunctionInfo(FunctionInfo&&) = default;
			
			FunctionInfo& operator=(FunctionInfo&&) = default;
		};
		
	}
	
}

#endif
