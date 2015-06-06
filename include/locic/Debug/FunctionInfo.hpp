#ifndef LOCIC_DEBUG_FUNCTIONINFO_HPP
#define LOCIC_DEBUG_FUNCTIONINFO_HPP

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Support/Name.hpp>

namespace locic {

	namespace Debug {
		
		struct FunctionInfo {
			bool isDefinition;
			Name name;
			SourceLocation declLocation;
			SourceLocation scopeLocation;
			
			FunctionInfo()
			: isDefinition(false), name(), declLocation(SourceLocation::Null()), scopeLocation(SourceLocation::Null()) { }
			
			FunctionInfo(const FunctionInfo& other)
			: isDefinition(other.isDefinition),
			name(other.name.copy()),
			declLocation(other.declLocation),
			scopeLocation(other.scopeLocation) { }
			
			FunctionInfo(FunctionInfo&&) = default;
			
			FunctionInfo& operator=(FunctionInfo&&) = default;
		};
		
	}
	
}

#endif
