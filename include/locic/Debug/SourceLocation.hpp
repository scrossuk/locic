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
					const auto nullPosition = SourcePosition(0, 0, 0);
					return SourceLocation(String::Null(), SourceRange(nullPosition, nullPosition),
						std::make_pair<size_t, size_t>(0, 0), std::make_pair<size_t, size_t>(0, 0));
				}
				
				SourceLocation(String pFileName, SourceRange pRange,
						std::pair<size_t, size_t> pByteRange,
						std::pair<size_t, size_t> pLineByteRange)
					: fileName_(std::move(pFileName)), range_(pRange),
					byteRange_(pByteRange), lineByteRange_(pLineByteRange) { }
					
				String fileName() const {
					return fileName_;
				}
				
				SourceRange range() const {
					return range_;
				}
				
				std::pair<size_t, size_t> byteRange() const {
					return byteRange_;
				}
				
				std::pair<size_t, size_t> lineByteRange() const {
					return lineByteRange_;
				}
				
				std::string toString() const {
					return makeString("SourceLocation(filename = %s, range = %s)",
						fileName_.c_str(), range_.toShortString().c_str());
				}
				
			private:
				String fileName_;
				SourceRange range_;
				std::pair<size_t, size_t> byteRange_;
				std::pair<size_t, size_t> lineByteRange_;
				
		};
		
	}
	
}

#endif
