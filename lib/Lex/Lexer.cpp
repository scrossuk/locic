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
					case 'n':
						stringLiteral.append('\n');
						break;
					case 't':
						stringLiteral.append('\t');
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
