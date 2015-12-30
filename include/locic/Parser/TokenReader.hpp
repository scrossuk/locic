#ifndef LOCIC_PARSER_TOKENREADER_HPP
#define LOCIC_PARSER_TOKENREADER_HPP

#include <deque>

#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Support/Array.hpp>

namespace locic {
	
	class String;
	
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
			
			Token expectOneOf(const Array<Token::Kind, 4>& tokenKinds);
			
			String expectName();
			
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