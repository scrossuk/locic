#ifndef LOCIC_LEX_CHARACTERREADER_HPP
#define LOCIC_LEX_CHARACTERREADER_HPP

#include <locic/Debug/SourcePosition.hpp>
#include <locic/Lex/Character.hpp>

namespace locic {
	
	namespace Lex {
		
		class CharacterSource;
		
		class CharacterReader {
		public:
			CharacterReader(CharacterSource& source);
			
			bool isEnd() const;
			
			Character get();
			
			Character peek();
			
			void consume();
			
			void expect(Character character);
			
			Debug::SourcePosition position() const;
			
		private:
			CharacterSource& source_;
			Character currentCharacter_;
			Debug::SourcePosition position_;
			
		};
		
	}
	
}

#endif