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
		class Token;
		
		class StringLiteralLexer {
		public:
			StringLiteralLexer(CharacterReader& reader);
			
			~StringLiteralLexer();
			
			Token lexCharacterLiteral();
			
			Token lexStringLiteral(const StringHost& stringHost);
			
			Character lexCharacter();
			
			Character lexEscapeSequence();
			
			Character lexOctalEscapeSequenceSuffix(Debug::SourcePosition sequencePosition);
			
			Character lexSymbolEscapeSequenceSuffix(Debug::SourcePosition sequencePosition);
			
		private:
			CharacterReader& reader_;
			
		};
		
	}
	
}

#endif