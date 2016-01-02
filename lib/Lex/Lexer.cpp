#include <locic/Debug/SourcePosition.hpp>
#include <locic/Debug/SourceRange.hpp>
#include <locic/Lex/Character.hpp>
#include <locic/Lex/CharacterReader.hpp>
#include <locic/Lex/CharacterSource.hpp>
#include <locic/Lex/Diagnostics.hpp>
#include <locic/Lex/DiagnosticReceiver.hpp>
#include <locic/Lex/IdentifierLexer.hpp>
#include <locic/Lex/Lexer.hpp>
#include <locic/Lex/NumericValue.hpp>
#include <locic/Lex/StringLiteralLexer.hpp>
#include <locic/Support/Array.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/StringBuilder.hpp>
#include <locic/Support/StringHost.hpp>

namespace locic {
	
	namespace Lex {
		
		Lexer::Lexer(CharacterSource& source, DiagnosticReceiver& diagnosticReceiver)
		: reader_(source), diagnosticReceiver_(diagnosticReceiver) { }
		
		Lexer::~Lexer() { }
		
		void Lexer::issueWarning(const Diag kind, const Debug::SourcePosition startPosition,
		                         const Debug::SourcePosition endPosition) {
			const Debug::SourceRange sourceRange(startPosition,
			                                     endPosition);
			diagnosticReceiver_.issueWarning(kind, sourceRange);
		}
		
		void Lexer::issueError(const Diag kind, const Debug::SourcePosition startPosition,
		                       const Debug::SourcePosition endPosition) {
			const Debug::SourceRange sourceRange(startPosition,
			                                     endPosition);
			diagnosticReceiver_.issueError(kind, sourceRange);
		}
		
		Token::Kind Lexer::getSymbolTokenKind(const Character value) {
			switch (value.value()) {
				case '%': return Token::Kind::PERCENT;
				case '=': return Token::Kind::SETEQUAL;
				case '+': return Token::Kind::PLUS;
				case '-': return Token::Kind::MINUS;
				case '*': return Token::Kind::STAR;
				case '/': return Token::Kind::FORWARDSLASH;
				case '!': return Token::Kind::EXCLAIMMARK;
				case '&': return Token::Kind::AMPERSAND;
				case '|': return Token::Kind::VERTICAL_BAR;
				case '?': return Token::Kind::QUESTIONMARK;
				case '@': return Token::Kind::AT;
				case ',': return Token::Kind::COMMA;
				case ':': return Token::Kind::COLON;
				case ';': return Token::Kind::SEMICOLON;
				case '(': return Token::Kind::LROUNDBRACKET;
				case ')': return Token::Kind::RROUNDBRACKET;
				case '{': return Token::Kind::LCURLYBRACKET;
				case '}': return Token::Kind::RCURLYBRACKET;
				case '[': return Token::Kind::LSQUAREBRACKET;
				case ']': return Token::Kind::RSQUAREBRACKET;
				case '.': return Token::Kind::DOT;
				case '<': return Token::Kind::LTRIBRACKET;
				case '>': return Token::Kind::RTRIBRACKET;
				case '~': return Token::Kind::TILDA;
				default: return Token::Kind::UNKNOWN;
			}
		}
		
		constexpr uint64_t JOIN(uint32_t a, uint32_t b) {
			return (static_cast<uint64_t>(a) << 32) | static_cast<uint64_t>(b);
		}
		
		Token::Kind Lexer::getDoubleSymbolTokenKind(const Character first, const Character second) {
			switch (JOIN(first.value(), second.value())) {
				case JOIN('=', '='): return Token::Kind::ISEQUAL;
				case JOIN('!', '='): return Token::Kind::NOTEQUAL;
				case JOIN('>', '='): return Token::Kind::GREATEROREQUAL;
				case JOIN('<', '='): return Token::Kind::LESSOREQUAL;
				case JOIN('-', '>'): return Token::Kind::PTRACCESS;
				case JOIN('+', '='): return Token::Kind::ADDEQUAL;
				case JOIN('-', '='): return Token::Kind::SUBEQUAL;
				case JOIN('*', '='): return Token::Kind::MULEQUAL;
				case JOIN('/', '='): return Token::Kind::DIVEQUAL;
				case JOIN('%', '='): return Token::Kind::PERCENTEQUAL;
				case JOIN('+', '+'): return Token::Kind::DOUBLE_PLUS;
				case JOIN('-', '-'): return Token::Kind::DOUBLE_MINUS;
				case JOIN('&', '&'): return Token::Kind::DOUBLE_AMPERSAND;
				case JOIN('|', '|'): return Token::Kind::DOUBLE_VERTICAL_BAR;
				case JOIN(':', ':'): return Token::Kind::DOUBLE_COLON;
				case JOIN('<', '<'): return Token::Kind::DOUBLE_LTRIBRACKET;
				default: return Token::Kind::UNKNOWN;
			}
		}
		
		Token Lexer::lexToken() {
			while (true) {
				if (reader_.isEnd()) {
					return Token::Basic(Token::END);
				}
				
				const auto startPosition = reader_.position();
				auto token = lexTokenWithoutLocation(reader_.stringHost());
				if (!token) {
					continue;
				}
				
				const auto endPosition = reader_.position();
				const auto range = Debug::SourceRange(startPosition,
				                                      endPosition);
				token->setSourceRange(range);
				return *token;
			}
		}
		
		Optional<Token> Lexer::lexTokenWithoutLocation(const StringHost& stringHost) {
			assert(!reader_.isEnd());
			
			const auto nextValue = reader_.peek();
			if (nextValue.isSpace()) {
				auto spaceValue = nextValue;
				while (spaceValue.isSpace()) {
					reader_.consume();
					spaceValue = reader_.peek();
				}
				return Optional<Token>();
			}
			
			if (nextValue.isAlpha() || nextValue == '_') {
				return make_optional(lexNameToken(stringHost));
			} else if (nextValue.isDigit()) {
				return make_optional(lexNumericToken());
			} else if (nextValue == '\'') {
				return make_optional(lexCharacterLiteral());
			} else if (nextValue == '"') {
				return make_optional(lexStringLiteral(stringHost));
			}
			
			reader_.consume();
			const auto nextNextValue = reader_.peek();
			
			const auto doubleSymbolKind = getDoubleSymbolTokenKind(nextValue,
			                                                       nextNextValue);
			if (doubleSymbolKind != Token::Kind::UNKNOWN) {
				reader_.consume();
				return make_optional(Token::Basic(doubleSymbolKind));
			}
			
			if (nextValue == '/' && (nextNextValue == '*' || nextNextValue == '/')) {
				lexComment();
				return Optional<Token>();
			}
			
			return make_optional(Token::Basic(getSymbolTokenKind(nextValue)));
		}
		
		Token Lexer::lexCharacterLiteral() {
			StringLiteralLexer stringLiteralLexer(reader_, diagnosticReceiver_);
			return stringLiteralLexer.lexCharacterLiteral();
		}
		
		Token Lexer::lexStringLiteral(const StringHost& stringHost) {
			StringLiteralLexer stringLiteralLexer(reader_, diagnosticReceiver_);
			return stringLiteralLexer.lexStringLiteral(stringHost);
		}
		
		NumericValue getHexIntegerConstant(const Array<Character, 16>& digits) {
			std::string data = "";
			
			for (const auto value: digits) {
				data += value.asciiValue();
			}
			
			return NumericValue::Integer(strtoll(data.c_str(), NULL, 16));
		}
		
		NumericValue getOctalIntegerConstant(const Array<Character, 16>& digits) {
			std::string data = "";
			
			for (const auto value: digits) {
				data += value.asciiValue();
			}
			
			return NumericValue::Integer(strtoll(data.c_str(), NULL, 8));
		}
		
		NumericValue getIntegerConstant(const Array<Character, 16>& digits) {
			std::string data;
			
			for (const auto value: digits) {
				data += value.asciiValue();
			}
			
			return NumericValue::Integer(strtoll(data.c_str(), NULL, 10));
		}
		
		NumericValue getFloatConstant(const Array<Character, 16>& digits) {
			std::string data;
			
			for (const auto value: digits) {
				data += value.asciiValue();
			}
			
			return NumericValue::Float(atof(data.c_str()));
		}
		
		NumericValue getVersionConstant(const Array<Character, 16>& digits) {
			std::string data;
			
			for (const auto value: digits) {
				data += value.asciiValue();
			}
			
			return NumericValue::Version(Version::FromString(data));
		}
		
		Token Lexer::lexNumericToken() {
			const auto numericValue = lexNumericConstant();
			switch (numericValue.kind()) {
				case NumericValue::INTEGER:
					return Token::Constant(Constant::Integer(numericValue.integerValue()));
				case NumericValue::FLOAT:
					return Token::Constant(Constant::Float(numericValue.floatValue()));
				case NumericValue::VERSION:
					return Token::Version(numericValue.versionValue());
			}
		}
		
		NumericValue Lexer::lexNumericConstant() {
			const auto start = reader_.position();
			
			Array<Character, 16> digits;
			
			const auto startDigit = reader_.peek();
			if (startDigit == '0') {
				reader_.consume();
				digits.push_back(startDigit);
				
				if (reader_.peek() == 'x') {
					digits.push_back(reader_.get());
					while (reader_.peek().isHexDigit()) {
						digits.push_back(reader_.get());
					}
					return getHexIntegerConstant(digits);
				} else if (reader_.peek().isDigit()) {
					while (true) {
						const auto next = reader_.peek();
						if (!next.isDigit()) {
							break;
						}
						reader_.consume();
						if (!next.isOctalDigit()) {
							issueError(Diag::InvalidOctalCharacter, start,
							           reader_.position());
						}
						digits.push_back(next);
					}
					
					return getOctalIntegerConstant(digits);
				}
			}
			
			while (reader_.peek().isDigit()) {
				digits.push_back(reader_.get());
			}
			
			if (reader_.peek() != '.') {
				return getIntegerConstant(digits);
			}
			
			reader_.consume();
			digits.push_back('.');
			
			while (reader_.peek().isDigit()) {
				digits.push_back(reader_.get());
			}
			
			if (reader_.peek() != '.') {
				return getFloatConstant(digits);
			}
			
			reader_.expect('.');
			digits.push_back('.');
			while (reader_.peek().isDigit()) {
				digits.push_back(reader_.get());
			}
			
			return getVersionConstant(digits);
		}
		
		Token Lexer::lexNameToken(const StringHost& stringHost) {
			IdentifierLexer identifierLexer(reader_, stringHost);
			return identifierLexer.lexIdentifier();
		}
		
		void Lexer::lexShortComment() {
			while (true) {
				const auto character = reader_.get();
				if (character.isNewline()) {
					break;
				}
			}
		}
		
		void Lexer::lexLongComment() {
			while (true) {
				const auto character = reader_.get();
				if (character == '*') {
					const auto next = reader_.get();
					if (next == '/') {
						break;
					}
				}
			}
		}
		
		void Lexer::lexComment() {
			const auto commentDefiningCharacter = reader_.get();
			assert(commentDefiningCharacter == '/' ||
			       commentDefiningCharacter == '*');
			
			if (commentDefiningCharacter == '/') {
				lexShortComment();
			} else {
				lexLongComment();
			}
		}
		
		String Lexer::fileName() const {
			return reader_.source().fileName();
		}
		
	}
	
}
