#include <cstdio>

#include <locic/Lex/Character.hpp>
#include <locic/Lex/CharacterSource.hpp>
#include <locic/Lex/FileCharacterSource.hpp>

namespace locic {
	
	namespace Lex {
		
		FileCharacterSource::FileCharacterSource(const String fileName,
		                                         FILE* const file)
		: fileName_(fileName), file_(file), position_(0) { }
		
		FileCharacterSource::~FileCharacterSource() { }
		
		Character FileCharacterSource::get() {
			const auto result = fgetc(file_);
			if (feof(file_)) {
				return Lex::Character(0);
			}
			position_++;
			return Lex::Character(result);
		}
		
		size_t FileCharacterSource::byteOffset() const {
			return position_;
		}
		
		String FileCharacterSource::fileName() const {
			return fileName_;
		}
		
	}
	
}
