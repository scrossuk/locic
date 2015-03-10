#ifndef LOCIC_DEBUG_SOURCEPOSITION_HPP
#define LOCIC_DEBUG_SOURCEPOSITION_HPP

#include <assert.h>
#include <stdint.h>

#include <string>

#include <locic/MakeString.hpp>

namespace locic {

	namespace Debug {
	
		class SourcePosition {
			public:
				SourcePosition(size_t pLineNumber, size_t pColumn)
				: lineNumber_(pLineNumber), column_(pColumn) { }
				
				size_t lineNumber() const {
					return lineNumber_;
				}
				
				size_t column() const {
					return column_;
				}
				
				bool operator<=(const SourcePosition& position) const {
					return (lineNumber() == position.lineNumber()) ?
						column() <= position.column() :
						lineNumber() < position.lineNumber();
				}
				
				bool operator<(const SourcePosition& position) const {
					return (lineNumber() == position.lineNumber()) ?
						column() < position.column() :
						lineNumber() < position.lineNumber();
				}
				
				std::string toString() const {
					return makeString("SourcePosition(lineNumber = %llu, column = %llu)",
						(unsigned long long) lineNumber_, (unsigned long long) column_);
				}
				
				std::string toShortString() const {
					return makeString("(line %llu, column %llu)",
						(unsigned long long) lineNumber_, (unsigned long long) column_);
				}
				
			private:
				size_t lineNumber_, column_;
				
		};
		
	}
	
}

#endif
