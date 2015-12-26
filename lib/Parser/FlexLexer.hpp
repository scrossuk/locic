#ifndef LOCIC_PARSER_FLEXLEXER_HPP
#define LOCIC_PARSER_FLEXLEXER_HPP

#include <cstdio>

#include <locic/Parser/Context.hpp>

#include "LexerAPI.hpp"
#include "LocationInfo.hpp"
#include "Token.hpp"

namespace locic {
	
	namespace Parser {
		
		void * LexAlloc(FILE * file, Context * context);
		
		int LexGetToken(Token* token, LocationInfo* locationInfo, void * scanner);
		
		void LexFree(void * scanner);
		
		class FlexLexer: public LexerAPI {
		public:
			FlexLexer(FILE * file, Context& context) {
				lexer_ = LexAlloc(file, &context);
			}
			
			~FlexLexer() {
				LexFree(lexer_);
			}
			
			FlexLexer(const FlexLexer&) = delete;
			FlexLexer& operator=(const FlexLexer&) = delete;
			
			FlexLexer(FlexLexer&&) = delete;
			FlexLexer& operator=(FlexLexer&&) = delete;
			
			int getToken(Token* token, LocationInfo* position) {
				return LexGetToken(token, position, lexer_);
			}
			
		private:
			void* lexer_;
			
		};
		
	}
	
}

#endif
