#ifndef LOCIC_LEX_CHARACTERSOURCE_HPP
#define LOCIC_LEX_CHARACTERSOURCE_HPP

namespace locic {
	
	namespace Lex {
		
		class Character;
		
		class CharacterSource {
		protected:
			~CharacterSource() { }
			
		public:
			virtual Character get() = 0;
			
		};
		
	}
	
}

#endif