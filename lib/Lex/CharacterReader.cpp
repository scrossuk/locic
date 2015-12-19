#include <locic/Lex/Character.hpp>
#include <locic/Lex/CharacterReader.hpp>
#include <locic/Lex/CharacterSource.hpp>

namespace locic {
	
	namespace Lex {
		
		CharacterReader::CharacterReader(CharacterSource& source)
		: source_(source), currentCharacter_(source.get()) { }
		
		bool CharacterReader::isEnd() const {
			return currentCharacter_ == 0;
		}
		
		Character CharacterReader::get() {
			assert(!isEnd());
			const auto currentCharacter = currentCharacter_;
			consume();
			return currentCharacter;
		}
		
		Character CharacterReader::peek() {
			return currentCharacter_;
		}
		
		void CharacterReader::consume() {
			currentCharacter_ = source_.get();
		}
		
		void CharacterReader::expect(const Character character) {
			assert(currentCharacter_ == character);
			consume();
		}
		
	}
	
}