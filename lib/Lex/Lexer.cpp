#include <locic/Lex/Character.hpp>
#include <locic/Lex/CharacterReader.hpp>
#include <locic/Lex/CharacterSource.hpp>
#include <locic/Lex/Diagnostics.hpp>
#include <locic/Lex/DiagnosticReceiver.hpp>
#include <locic/Lex/Lexer.hpp>
#include <locic/Lex/NumericValue.hpp>
#include <locic/Support/Array.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/StringBuilder.hpp>
#include <locic/Support/StringHost.hpp>

namespace locic {
	
	namespace Lex {
		
		Lexer::Lexer(CharacterSource& source, DiagnosticReceiver& diagnosticReceiver)
		: reader_(source), diagnosticReceiver_(diagnosticReceiver) { }
		
		Lexer::~Lexer() { }
		
		void Lexer::issueWarning(const Diag kind) {
			diagnosticReceiver_.issueWarning(kind);
		}
		
		void Lexer::issueError(const Diag kind) {
			diagnosticReceiver_.issueError(kind);
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
		
		String Lexer::lexStringLiteral(const StringHost& stringHost) {
			StringBuilder stringLiteral(stringHost);
			reader_.expect('"');
			while (true) {
				if (reader_.isEnd()) {
					issueError(Diag::UnterminatedStringLiteral);
					return stringLiteral.getString();
				}
				
				const auto next = reader_.get();
				if (next == '"') {
					return stringLiteral.getString();
				}
				
				if (next != '\\') {
					stringLiteral.append(next.asciiValue());
					continue;
				}
				
				if (reader_.isEnd()) {
					issueError(Diag::UnterminatedStringLiteral);
					stringLiteral.append(next.asciiValue());
					return stringLiteral.getString();
				}
				
				const auto escapeChar = reader_.get();
				
				switch (escapeChar.value()) {
					case '\\':
						stringLiteral.append('\\');
						break;
					case '"':
						stringLiteral.append('"');
						break;
					case 'a':
						stringLiteral.append('\a');
						break;
					case 'b':
						stringLiteral.append('\b');
						break;
					case 'f':
						stringLiteral.append('\f');
						break;
					case 'n':
						stringLiteral.append('\n');
						break;
					case 'r':
						stringLiteral.append('\r');
						break;
					case 't':
						stringLiteral.append('\t');
						break;
					case 'v':
						stringLiteral.append('\v');
						break;
					default:
						issueError(Diag::InvalidStringLiteralEscape);
						stringLiteral.append('\\');
						stringLiteral.append(escapeChar.asciiValue());
						break;
				}
			}
		}
		
		NumericValue getHexIntegerConstant(const Array<Character, 16>& digits) {
			std::string data = "0x";
			
			for (const auto value: digits) {
				data += value.asciiValue();
			}
			
			return NumericValue::Integer(strtoll(data.c_str(), NULL, 16));
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
			Array<Character, 16> digits;
			
			const auto startDigit = reader_.peek();
			if (startDigit == '0') {
				reader_.consume();
				if (reader_.peek() == 'x') {
					reader_.consume();
					while (reader_.peek().isHexDigit()) {
						digits.push_back(reader_.get());
					}
					return getHexIntegerConstant(digits);
				}
				digits.push_back(startDigit);
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
		
	}
	
}
