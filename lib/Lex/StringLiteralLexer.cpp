#include <locic/Debug/SourcePosition.hpp>
#include <locic/Debug/SourceRange.hpp>
#include <locic/Frontend/DiagnosticReceiver.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Lex/Character.hpp>
#include <locic/Lex/CharacterReader.hpp>
#include <locic/Lex/Diagnostics.hpp>
#include <locic/Lex/StringLiteralLexer.hpp>
#include <locic/Lex/Token.hpp>
#include <locic/Support/Array.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/StringBuilder.hpp>
#include <locic/Support/StringHost.hpp>

namespace locic {
	
	namespace Lex {
		
		StringLiteralLexer::StringLiteralLexer(CharacterReader& reader)
		: reader_(reader) { }
		
		StringLiteralLexer::~StringLiteralLexer() { }
		
		Token StringLiteralLexer::lexCharacterLiteral() {
			const auto start = reader_.position();
			
			Array<Character, 4> characters;
			
			reader_.expect('\'');
			while (true) {
				if (reader_.isEnd()) {
					reader_.issueDiag(UnterminatedCharacterLiteralDiag(), start);
					const auto value = characters.empty() ? 0 : characters.front().value();
					return Token::Constant(Constant::Character(value));
				}
				
				if (reader_.peek() == '\'') {
					reader_.consume();
					break;
				}
				
				const auto character = lexCharacter();
				characters.push_back(character);
			}
			
			if (characters.empty()) {
				reader_.issueDiag(EmptyCharacterLiteralDiag(), start);
				return Token::Constant(Constant::Character(0));
			}
			
			if (characters.size() > 1) {
				reader_.issueDiag(MultiCharCharacterLiteralDiag(), start);
			}
			
			return Token::Constant(Constant::Character(characters[0].value()));
		}
		
		Token StringLiteralLexer::lexStringLiteral(const StringHost& stringHost) {
			const auto start = reader_.position();
			
			StringBuilder stringLiteral(stringHost);
			reader_.expect('"');
			while (true) {
				if (reader_.isEnd()) {
					reader_.issueDiag(UnterminatedStringLiteralDiag(), start);
					return Token::Constant(Constant::StringVal(stringLiteral.getString()));
				}
				
				if (reader_.peek() == '"') {
					reader_.consume();
					return Token::Constant(Constant::StringVal(stringLiteral.getString()));
				}
				
				const auto character = lexCharacter();
				stringLiteral.append(character.asciiValue());
			}
		}
		
		Character StringLiteralLexer::lexCharacter() {
			assert(!reader_.isEnd());
			const auto next = reader_.peek();
			
			if (next == '\\') {
				return lexEscapeSequence();
			} else {
				reader_.consume();
				return next;
			}
		}
		
		Character StringLiteralLexer::lexEscapeSequence() {
			const auto start = reader_.position();
			
			reader_.expect('\\');
			
			if (reader_.isEnd()) {
				reader_.issueDiag(InvalidStringLiteralEscapeDiag(), start);
				return Character('\\');
			}
			
			const auto firstEscapeChar = reader_.peek();
			
			if (firstEscapeChar.isOctalDigit()) {
				return lexOctalEscapeSequenceSuffix(start);
			} else {
				return lexSymbolEscapeSequenceSuffix(start);
			}
		}
		
		Character StringLiteralLexer::lexOctalEscapeSequenceSuffix(const Debug::SourcePosition sequencePosition) {
			assert(reader_.peek().isOctalDigit());
			
			std::string octalDigits;
			
			while (true) {
				const auto escapeChar = reader_.peek();
				if (!escapeChar.isOctalDigit()) {
					break;
				}
				octalDigits += escapeChar.asciiValue();
				reader_.consume();
			}
			
			auto value = strtoll(octalDigits.c_str(), NULL, 8);
			if (value > 255) {
				reader_.issueDiag(OctalEscapeSequenceOutOfRangeDiag(), sequencePosition);
				value = 0;
			}
			
			return Character(value);
		}
		
		Character StringLiteralLexer::lexSymbolEscapeSequenceSuffix(const Debug::SourcePosition sequencePosition) {
			switch (reader_.peek().value()) {
				case '\\':
					reader_.consume();
					return Character('\\');
				case '"':
					reader_.consume();
					return Character('"');
				case '\'':
					reader_.consume();
					return Character('\'');
				case 'a':
					reader_.consume();
					return Character('\a');
				case 'b':
					reader_.consume();
					return Character('\b');
				case 'f':
					reader_.consume();
					return Character('\f');
				case 'n':
					reader_.consume();
					return Character('\n');
				case 'r':
					reader_.consume();
					return Character('\r');
				case 't':
					reader_.consume();
					return Character('\t');
				case 'v':
					reader_.consume();
					return Character('\v');
				default:
					reader_.issueDiag(InvalidStringLiteralEscapeDiag(),
					                  sequencePosition);
					return Character('\\');
			}
		}
		
	}
	
}
