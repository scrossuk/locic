#include <locic/Debug/SourcePosition.hpp>
#include <locic/Debug/SourceRange.hpp>
#include <locic/Lex/Character.hpp>
#include <locic/Lex/CharacterReader.hpp>
#include <locic/Lex/DiagnosticReceiver.hpp>
#include <locic/Lex/Diagnostics.hpp>
#include <locic/Lex/StringLiteralLexer.hpp>
#include <locic/Lex/Token.hpp>
#include <locic/Support/Array.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/StringBuilder.hpp>
#include <locic/Support/StringHost.hpp>

namespace locic {
	
	namespace Lex {
		
		StringLiteralLexer::StringLiteralLexer(CharacterReader& reader,
		                                       DiagnosticReceiver& diagnosticReceiver)
		: reader_(reader), diagnosticReceiver_(diagnosticReceiver) { }
		
		StringLiteralLexer::~StringLiteralLexer() { }
		
		void StringLiteralLexer::issueWarning(const Diag kind,
		                                      const Debug::SourcePosition startPosition,
		                                      const Debug::SourcePosition endPosition) {
			const Debug::SourceRange sourceRange(startPosition,
			                                     endPosition);
			diagnosticReceiver_.issueWarning(kind, sourceRange);
		}
		
		void StringLiteralLexer::issueError(const Diag kind,
		                                    const Debug::SourcePosition startPosition,
		                                    const Debug::SourcePosition endPosition) {
			const Debug::SourceRange sourceRange(startPosition,
			                                     endPosition);
			diagnosticReceiver_.issueError(kind, sourceRange);
		}
		
		Token StringLiteralLexer::lexStringLiteral(const StringHost& stringHost) {
			const auto start = reader_.position();
			
			StringBuilder stringLiteral(stringHost);
			reader_.expect('"');
			while (true) {
				if (reader_.isEnd()) {
					issueError(Diag::UnterminatedStringLiteral, start,
					           reader_.position());
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
			const auto charPosition = reader_.position();
			
			reader_.expect('\\');
			
			if (reader_.isEnd()) {
				issueError(Diag::InvalidStringLiteralEscape,
				           charPosition, reader_.position());
				return Character('\\');
			}
			
			return lexSymbolEscapeSequenceSuffix(charPosition);
		}
		
		Character StringLiteralLexer::lexSymbolEscapeSequenceSuffix(const Debug::SourcePosition sequencePosition) {
			switch (reader_.peek().value()) {
				case '\\':
					reader_.consume();
					return Character('\\');
				case '"':
					reader_.consume();
					return Character('"');
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
					issueError(Diag::InvalidStringLiteralEscape,
					           sequencePosition,
					           reader_.position());
					return Character('\\');
			}
		}
		
	}
	
}
