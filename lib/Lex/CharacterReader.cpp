#include <locic/Debug/SourcePosition.hpp>
#include <locic/Lex/Character.hpp>
#include <locic/Lex/CharacterReader.hpp>
#include <locic/Lex/CharacterSource.hpp>

namespace locic {
	
	namespace Lex {
		
		CharacterReader::CharacterReader(CharacterSource& source)
		: source_(source), currentCharacter_(0),
		position_(1, 1, source.byteOffset()) {
			currentCharacter_ = source.get();
		}
		
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
			if (currentCharacter_.isNewline()) {
				position_ = Debug::SourcePosition(/*lineNumber=*/position_.lineNumber() + 1,
				                                  /*column=*/1,
				                                  /*byteOffset=*/source_.byteOffset());
			} else {
				position_ = Debug::SourcePosition(/*lineNumber=*/position_.lineNumber(),
				                                  /*column=*/position_.column() + 1,
				                                  /*byteOffset=*/source_.byteOffset());
			}
			currentCharacter_ = source_.get();
		}
		
		void CharacterReader::expect(const Character character) {
			(void) character;
			assert(currentCharacter_ == character);
			consume();
		}
		
		Debug::SourcePosition CharacterReader::position() const {
			return position_;
		}
		
	}
	
}