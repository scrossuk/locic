#ifndef LOCIC_LEX_CHARACTERREADER_HPP
#define LOCIC_LEX_CHARACTERREADER_HPP

#include <locic/Debug/SourcePosition.hpp>
#include <locic/Lex/Character.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Lex {
		
		class CharacterSource;
		
		class CharacterReader {
		public:
			CharacterReader(CharacterSource& source);
			
			const StringHost& stringHost() const;
			
			CharacterSource& source();
			const CharacterSource& source() const;
			
			bool isEnd() const;
			
			Character get();
			
			Character peek();
			
			void consume();
			
			void expect(Character character);
			
			Debug::SourcePosition position() const;
			
		private:
			const StringHost& stringHost_;
			CharacterSource& source_;
			Character currentCharacter_;
			Debug::SourcePosition position_;
			
		};
		
	}
	
}

#endif