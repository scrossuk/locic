#ifndef MOCKCHARACTERSOURCE_HPP
#define MOCKCHARACTERSOURCE_HPP

#include <locic/Lex/Character.hpp>
#include <locic/Lex/CharacterSource.hpp>
#include <locic/Support/Array.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/StringHost.hpp>

class MockCharacterSource: public locic::Lex::CharacterSource {
public:
	MockCharacterSource(const locic::StringHost& stringHost,
	                    locic::Array<locic::Lex::Character, 16> characters)
	: stringHost_(stringHost), characters_(characters.copy()), position_(0) { }
	
	locic::Lex::Character get() {
		assert(position_ <= characters_.size());
		if (position_ < characters_.size()) {
			return characters_[position_++];
		} else {
			position_++;
			return 0;
		}
	}
	
	size_t byteOffset() const {
		return position_;
	}
	
	bool empty() const {
		return position_ == (characters_.size() + 1);
	}
	
	locic::String fileName() const {
		return locic::String(stringHost_, "<test>");
	}
	
private:
	const locic::StringHost& stringHost_;
	locic::Array<locic::Lex::Character, 16> characters_;
	size_t position_;
	
};

#endif