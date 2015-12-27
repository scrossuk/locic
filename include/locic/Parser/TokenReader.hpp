#ifndef LOCIC_PARSER_TOKENREADER_HPP
#define LOCIC_PARSER_TOKENREADER_HPP

#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/Token.hpp>

namespace locic {
	
	namespace Parser {
		
		class TokenSource;
		
		class TokenReader {
		public:
			TokenReader(TokenSource& source);
			
			bool isEnd() const;
			
			Token get();
			
			Token peek();
			
			void consume();
			
			void expect(Token::Kind tokenKind);
			
			Debug::SourcePosition position() const;
			
		private:
			TokenSource& source_;
			Token currentToken_;
			Debug::SourcePosition position_;
			
		};
		
	}
	
}

#endif