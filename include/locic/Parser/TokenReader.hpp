#ifndef LOCIC_PARSER_TOKENREADER_HPP
#define LOCIC_PARSER_TOKENREADER_HPP

#include <deque>

#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/Token.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourceLocation;
		class SourceRange;
		
	}
	
	namespace Parser {
		
		class TokenSource;
		
		class TokenReader {
		public:
			TokenReader(TokenSource& source);
			
			bool isEnd() const;
			
			Token get();
			
			Token peek(size_t offset = 0);
			
			void consume();
			
			void expect(Token::Kind tokenKind);
			
			Debug::SourcePosition position() const;
			
			Debug::SourcePosition lastTokenEndPosition() const;
			
			Debug::SourceRange rangeFrom(Debug::SourcePosition start) const;
			
			Debug::SourceLocation locationWithRangeFrom(Debug::SourcePosition start) const;
			
		private:
			TokenSource& source_;
			std::deque<Token> tokens_;
			Debug::SourcePosition position_;
			Debug::SourcePosition lastEndPosition_;
			
		};
		
	}
	
}

#endif