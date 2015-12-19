#ifndef LOCIC_LEX_CHARACTERREADER_HPP
#define LOCIC_LEX_CHARACTERREADER_HPP

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
			
		private:
			CharacterSource& source_;
			Character currentCharacter_;
			
		};
		
	}
	
}

#endif