#ifndef LOCIC_DEBUG_SOURCEPOSITION_HPP
#define LOCIC_DEBUG_SOURCEPOSITION_HPP

#include <stdint.h>

#include <cassert>
#include <string>

#include <locic/Support/MakeString.hpp>

namespace locic {

	namespace Debug {
	
		class SourcePosition {
			public:
				SourcePosition(size_t pLineNumber, size_t pColumn,
				               size_t pByteOffset)
				: lineNumber_(pLineNumber), column_(pColumn),
				byteOffset_(pByteOffset) {
				}
				
				size_t lineNumber() const {
					return lineNumber_;
				}
				
				size_t column() const {
					return column_;
				}
				
				size_t byteOffset() const {
					return byteOffset_;
				}
				
				bool isNull() const {
					return lineNumber() == 0 &&
					       column() == 0 &&
					       byteOffset() == 0;
				}
				
				bool operator==(const SourcePosition& position) const {
					checkComparisonInvariants(position);
					return byteOffset() == position.byteOffset();
				}
				
				bool operator!=(const SourcePosition& position) const {
					return !(*this == position);
				}
				
				bool operator<=(const SourcePosition& position) const {
					checkComparisonInvariants(position);
					return byteOffset() <= position.byteOffset();
				}
				
				bool operator<(const SourcePosition& position) const {
					checkComparisonInvariants(position);
					return byteOffset() < position.byteOffset();
				}
				
				std::string toString() const {
					return makeString("SourcePosition(lineNumber = %llu, column = %llu, byteOffset = %llu)",
					                  (unsigned long long) lineNumber_,
					                  (unsigned long long) column_,
					                  (unsigned long long) byteOffset_);
				}
				
				std::string toShortString() const {
					return makeString("(line %llu, column %llu)",
					                  (unsigned long long) lineNumber_,
					                  (unsigned long long) column_);
				}
				
			private:
				void checkComparisonInvariants(const SourcePosition& position) const {
					if (lineNumber() < position.lineNumber()) {
						assert(byteOffset() < position.byteOffset());
					} else if (lineNumber() == position.lineNumber()) {
						if (column() < position.column()) {
							assert(byteOffset() < position.byteOffset());
						} else if (column() == position.column()) {
							assert(byteOffset() == position.byteOffset());
						} else {
							assert(byteOffset() > position.byteOffset());
						}
					} else {
						assert(byteOffset() > position.byteOffset());
					}
				}
				
				size_t lineNumber_, column_, byteOffset_;
				
		};
		
	}
	
}

#endif
