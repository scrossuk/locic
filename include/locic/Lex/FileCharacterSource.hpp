#ifndef LOCIC_LEX_FILECHARACTERSOURCE_HPP
#define LOCIC_LEX_FILECHARACTERSOURCE_HPP

#include <cstdio>

#include <locic/Lex/CharacterSource.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace Lex {
		
		class Character;
		
		class FileCharacterSource: public CharacterSource {
		public:
			FileCharacterSource(String fileName, FILE* file);
			~FileCharacterSource();
			
			Character get();
			
			size_t byteOffset() const;
			
			String fileName() const;
			
		private:
			String fileName_;
			FILE * file_;
			size_t position_;
			
		};
		
	}
	
}

#endif