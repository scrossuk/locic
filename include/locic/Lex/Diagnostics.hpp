#ifndef LOCIC_LEX_DIAGNOSTICS_HPP
#define LOCIC_LEX_DIAGNOSTICS_HPP

#include <locic/Frontend/Diagnostics.hpp>

namespace locic {
	
	namespace Lex {
		
		enum class DiagID {
			UnterminatedStringLiteral,
			InvalidStringLiteralEscape,
			InvalidOctalCharacter,
			OctalEscapeSequenceOutOfRange,
			EmptyCharacterLiteral,
			UnterminatedCharacterLiteral,
			MultiCharCharacterLiteral
		};
		
#define LEXERROR(name, msg) \
	class name ## Diag: public Error { \
	public: \
		name ## Diag() { } \
		DiagID lexId() const { return DiagID::name; } \
		std::string toString() const { return msg; } \
	}
		
		LEXERROR(InvalidOctalCharacter, "invalid octal character");
		LEXERROR(EmptyCharacterLiteral, "character literal is empty");
		LEXERROR(MultiCharCharacterLiteral, "character literal contains multiple characters");
		LEXERROR(InvalidStringLiteralEscape, "invalid string literal escape");
		LEXERROR(OctalEscapeSequenceOutOfRange, "octal escape sequence is out of range");
		LEXERROR(UnterminatedCharacterLiteral, "unterminated character literal");
		LEXERROR(UnterminatedStringLiteral, "unterminated string literal");
		
	}
	
}

#endif