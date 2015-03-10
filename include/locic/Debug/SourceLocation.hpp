#ifndef LOCIC_DEBUG_SOURCELOCATION_HPP
#define LOCIC_DEBUG_SOURCELOCATION_HPP

#include <assert.h>
#include <stdint.h>

#include <string>
#include <utility>

#include <locic/Debug/SourcePosition.hpp>
#include <locic/Debug/SourceRange.hpp>
#include <locic/MakeString.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourceLocation {
			public:
				static SourceLocation Null() {
					const auto nullPosition = SourcePosition(0, 0);
					return SourceLocation("<NULL>", SourceRange(nullPosition, nullPosition),
						std::make_pair<size_t, size_t>(0, 0), std::make_pair<size_t, size_t>(0, 0));
				}
				
				SourceLocation(const std::string& pFileName, SourceRange pRange,
						std::pair<size_t, size_t> pByteRange,
						std::pair<size_t, size_t> pLineByteRange)
					: fileName_(pFileName), range_(pRange),
					byteRange_(pByteRange), lineByteRange_(pLineByteRange) { }
					
				const std::string& fileName() const {
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
				std::string fileName_;
				SourceRange range_;
				std::pair<size_t, size_t> byteRange_;
				std::pair<size_t, size_t> lineByteRange_;
				
		};
		
	}
	
}

#endif
