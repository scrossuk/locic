#ifndef LOCIC_PARSER_LEXERAPI_HPP
#define LOCIC_PARSER_LEXERAPI_HPP

#include "LocationInfo.hpp"
#include "Token.hpp"

namespace locic {
	
	namespace Parser {
		
		class LexerAPI {
		public:
			virtual int getToken(Token* token, LocationInfo* position) = 0;
			
		protected:
			~LexerAPI() { }
			
		};
		
	}
	
}

#endif
