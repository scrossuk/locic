#ifndef LOCIC_LEX_LEXER_HPP
#define LOCIC_LEX_LEXER_HPP

#include <locic/Lex/CharacterReader.hpp>
#include <locic/Lex/Token.hpp>

namespace locic {
	
	class Constant;
	class String;
	class StringHost;
	
	namespace Lex {
		
		class CharacterSource;
		enum class Diag;
		class DiagnosticReceiver;
		class NumericValue;
		
		class Lexer {
		public:
			Lexer(CharacterSource& source, DiagnosticReceiver& diagnosticReceiver);
			~Lexer();
			
			void issueWarning(Diag kind);
			
			void issueError(Diag kind);
			
			Token::Kind getSymbolTokenKind(Character value);
			
			Token::Kind getDoubleSymbolTokenKind(Character first,
			                                     Character second);
			
			String lexStringLiteral(const StringHost& stringHost);
			
			NumericValue lexNumericConstant();
			
		private:
			CharacterReader reader_;
			DiagnosticReceiver& diagnosticReceiver_;
			
		};
		
	}
	
}

#endif