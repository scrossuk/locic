#ifndef LOCIC_PARSER_LEXERAPI_HPP
#define LOCIC_PARSER_LEXERAPI_HPP

#include <locic/Lex/Lexer.hpp>

#include "LocationInfo.hpp"
#include "Token.hpp"

namespace locic {
	
	namespace Parser {
		
		class LexerAPI {
		public:
			virtual int getToken(GeneratedToken* token, LocationInfo* position) = 0;
			
			virtual Lex::Lexer& getLexer() = 0;
			
		protected:
			~LexerAPI() { }
			
		};
		
	}
	
}

#endif
