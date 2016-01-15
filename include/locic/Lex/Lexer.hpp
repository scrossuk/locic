#ifndef LOCIC_LEX_LEXER_HPP
#define LOCIC_LEX_LEXER_HPP

#include <locic/Lex/CharacterReader.hpp>
#include <locic/Lex/LexerAPI.hpp>
#include <locic/Lex/Token.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	class Constant;
	class DiagnosticReceiver;
	class String;
	class StringHost;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Lex {
		
		class CharacterSource;
		class NumericValue;
		
		class Lexer: public LexerAPI {
		public:
			Lexer(CharacterSource& source, DiagnosticReceiver& diagnosticReceiver);
			~Lexer();
			
			Token::Kind getSymbolTokenKind(Character value);
			
			Token::Kind getDoubleSymbolTokenKind(Character first,
			                                     Character second);
			
			Token lexToken();
			
			Optional<Token> lexTokenWithoutLocation(const StringHost& stringHost);
			
			Token lexCharacterLiteral();
			
			Token lexStringLiteral(const StringHost& stringHost);
			
			Token lexNumericToken();
			
			NumericValue lexNumericConstant();
			
			Token lexNameToken(const StringHost& stringHost);
			
			void lexShortComment();
			
			void lexLongComment();
			
			void lexComment();
			
			String fileName() const;
			
		private:
			CharacterReader reader_;
			
		};
		
	}
	
}

#endif