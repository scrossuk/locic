#ifndef LOCIC_LEX_CHARACTERSOURCE_HPP
#define LOCIC_LEX_CHARACTERSOURCE_HPP

namespace locic {
	
	class String;
	
	namespace Lex {
		
		class Character;
		
		class CharacterSource {
		protected:
			~CharacterSource() { }
			
		public:
			virtual Character get() = 0;
			
			virtual size_t byteOffset() const = 0;
			
			virtual String fileName() const = 0;
			
		};
		
	}
	
}

#endif