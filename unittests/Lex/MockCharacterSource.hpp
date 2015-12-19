#ifndef MOCKCHARACTERSOURCE_HPP
#define MOCKCHARACTERSOURCE_HPP

#include <locic/Lex/Character.hpp>
#include <locic/Lex/CharacterSource.hpp>
#include <locic/Support/Array.hpp>

class MockCharacterSource: public locic::Lex::CharacterSource {
public:
	MockCharacterSource(locic::Array<locic::Lex::Character, 16> characters)
	: characters_(characters.copy()), position_(0) { }
	
	locic::Lex::Character get() {
		assert(position_ <= characters_.size());
		if (position_ < characters_.size()) {
			return characters_[position_++];
		} else {
			position_++;
			return 0;
		}
	}
	
	bool empty() const {
		return position_ == (characters_.size() + 1);
	}
	
private:
	locic::Array<locic::Lex::Character, 16> characters_;
	size_t position_;
	
};

#endif