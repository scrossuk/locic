#ifndef LOCIC_DEBUG_SOURCELOCATION_HPP
#define LOCIC_DEBUG_SOURCELOCATION_HPP

#include <assert.h>
#include <stdint.h>

#include <string>
#include <utility>

#include <locic/Debug/SourcePosition.hpp>
#include <locic/Debug/SourceRange.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourceLocation {
			public:
				static SourceLocation Null() {
					return SourceLocation(String::Null(), SourceRange::Null());
				}
				
				SourceLocation(String pFileName, SourceRange pRange)
				: fileName_(std::move(pFileName)), range_(pRange) { }
				
				String fileName() const {
					return fileName_;
				}
				
				SourceRange range() const {
					return range_;
				}
				
				std::string toString() const {
					return makeString("SourceLocation(filename = %s, range = %s)",
						fileName_.c_str(), range_.toShortString().c_str());
				}
				
			private:
				String fileName_;
				SourceRange range_;
				
		};
		
	}
	
}

#endif
