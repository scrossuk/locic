#ifndef LOCIC_LEX_LEXERAPI_HPP
#define LOCIC_LEX_LEXERAPI_HPP

namespace locic {
	
	class String;
	class StringHost;
	
	namespace Lex {
		
		class Token;
		
		class LexerAPI {
		protected:
			~LexerAPI() { }
			
		public:
			virtual Token lexToken() = 0;
			
			virtual String fileName() const = 0;
			
		};
		
	}
	
}

#endif