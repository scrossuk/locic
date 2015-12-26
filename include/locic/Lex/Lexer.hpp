#ifndef LOCIC_LEX_LEXER_HPP
#define LOCIC_LEX_LEXER_HPP

#include <locic/Lex/CharacterReader.hpp>
#include <locic/Lex/Token.hpp>

namespace locic {
	
	class Constant;
	class String;
	class StringHost;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Lex {
		
		class CharacterSource;
		enum class Diag;
		class DiagnosticReceiver;
		class NumericValue;
		
		class Lexer {
		public:
			Lexer(CharacterSource& source, DiagnosticReceiver& diagnosticReceiver);
			~Lexer();
			
			void issueWarning(Diag kind, Debug::SourcePosition startPosition,
			                  Debug::SourcePosition endPosition);
			
			void issueError(Diag kind, Debug::SourcePosition startPosition,
			                Debug::SourcePosition endPosition);
			
			Token::Kind getSymbolTokenKind(Character value);
			
			Token::Kind getDoubleSymbolTokenKind(Character first,
			                                     Character second);
			
			Token lexToken(const StringHost& stringHost);
			
			Token lexStringLiteral(const StringHost& stringHost);
			
			Token lexNumericToken();
			
			NumericValue lexNumericConstant();
			
			Token lexNameToken(const StringHost& stringHost);
			
			void lexShortComment();
			
			void lexLongComment();
			
			void lexComment();
			
		private:
			CharacterReader reader_;
			DiagnosticReceiver& diagnosticReceiver_;
			
		};
		
	}
	
}

#endif