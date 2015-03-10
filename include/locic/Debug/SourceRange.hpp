#ifndef LOCIC_DEBUG_SOURCERANGE_HPP
#define LOCIC_DEBUG_SOURCERANGE_HPP

#include <assert.h>
#include <stdint.h>

#include <string>

#include <locic/Debug/SourcePosition.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace Debug {
	
		class SourceRange {
			public:
				SourceRange(SourcePosition pStart, SourcePosition pEnd)
				: start_(pStart), end_(pEnd) {
					assert(pStart <= pEnd);
				}
				
				SourcePosition start() const {
					return start_;
				}
				
				SourcePosition end() const {
					return end_;
				}
				
				std::string toString() const {
					return makeString("SourceRange(start = %s, end = %s)",
						start_.toShortString().c_str(), end_.toShortString().c_str());
				}
				
				std::string toShortString() const {
					return makeString("%s to %s",
						start_.toShortString().c_str(), end_.toShortString().c_str());
				}
				
			private:
				SourcePosition start_, end_;
				
		};
		
	}
	
}

#endif
