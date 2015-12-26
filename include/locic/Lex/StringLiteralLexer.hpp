#ifndef LOCIC_LEX_STRINGLITERALLEXER_HPP
#define LOCIC_LEX_STRINGLITERALLEXER_HPP

namespace locic {
	
	class StringHost;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Lex {
		
		class Character;
		class CharacterReader;
		class DiagnosticReceiver;
		class Token;
		
		class StringLiteralLexer {
		public:
			StringLiteralLexer(CharacterReader& reader,
			                   DiagnosticReceiver& diagnosticReceiver);
			
			~StringLiteralLexer();
			
			void issueWarning(Diag kind, Debug::SourcePosition startPosition,
			                  Debug::SourcePosition endPosition);
			
			void issueError(Diag kind, Debug::SourcePosition startPosition,
			                Debug::SourcePosition endPosition);
			
			Token lexStringLiteral(const StringHost& stringHost);
			
			Character lexCharacter();
			
			Character lexEscapeSequence();
			
			Character lexOctalEscapeSequenceSuffix(Debug::SourcePosition sequencePosition);
			
			Character lexSymbolEscapeSequenceSuffix(Debug::SourcePosition sequencePosition);
			
		private:
			CharacterReader& reader_;
			DiagnosticReceiver& diagnosticReceiver_;
			
		};
		
	}
	
}

#endif