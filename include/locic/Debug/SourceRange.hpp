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
				static SourceRange Null() {
					const auto nullPosition = SourcePosition(0, 0, 0);
					return SourceRange(nullPosition, nullPosition);
				}
				
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
				
				bool isNull() const {
					return start().isNull() &&
					       end().isNull();
				}
				
				bool operator==(const SourceRange& other) const {
					assert(!isNull() && !other.isNull());
					return start() == other.start() &&
					       end() == other.end();
				}
				
				bool operator!=(const SourceRange& other) const {
					assert(!isNull() && !other.isNull());
					return !(*this == other);
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
