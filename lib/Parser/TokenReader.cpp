#include <deque>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Debug/SourceRange.hpp>
#include <locic/Lex/LexerAPI.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace Parser {
		
		class UnexpectedTokenDiag: public Error {
		public:
			UnexpectedTokenDiag(Array<Token::Kind, 4> expected,
			                    const Token::Kind actual)
			: expected_(std::move(expected)), actual_(actual) {
				assert(!expected_.empty());
			}
			
			std::string toString() const {
				if (expected_.size() == 1) {
					return makeString("Expected '%s'; got '%s'.",
					                  Token::kindToString(expected_[0]).c_str(),
					                  Token::kindToString(actual_).c_str());
				} else if (expected_.size() > 3) {
					return makeString("Unexpected token '%s'.",
					                  Token::kindToString(actual_).c_str());
				}
				
				std::string expectStr = "{ ";
				for (size_t i = 0; i < expected_.size(); i++) {
					expectStr += Token::kindToString(expected_[i]);
					if ((i + 1) < expected_.size()) {
						expectStr += ", ";
					}
				}
				expectStr += " }";
				
				return makeString("Expected one of %s; got '%s'.",
				                  expectStr.c_str(), Token::kindToString(actual_).c_str());
			}
			
		private:
			Array<Token::Kind, 4> expected_;
			Token::Kind actual_;
			
		};
		
		TokenReader::TokenReader(Lex::LexerAPI& source, DiagnosticReceiver& diagReceiver)
		: source_(source), tokens_(1, source.lexToken()),
		position_(tokens_.front().sourceRange().start()),
		lastEndPosition_(position_), diagReceiver_(diagReceiver) { }
		
		String TokenReader::makeCString(const char* const string) const {
			return String(source_.fileName().host(), string);
		}
		
		bool TokenReader::isEnd() const {
			return tokens_.front().isEnd();
		}
		
		Token TokenReader::get() {
			assert(!tokens_.empty());
			assert(!isEnd());
			const auto currentToken = peek();
			consume();
			return currentToken;
		}
		
		Token TokenReader::peek(const size_t offset) {
			assert(!tokens_.empty());
			
			while (offset >= tokens_.size()) {
				assert(!tokens_.back().isEnd());
				tokens_.push_back(source_.lexToken());
			}
			
			return tokens_[offset];
		}
		
		void TokenReader::consume() {
			assert(!tokens_.empty());
			assert(!isEnd());
			lastEndPosition_ = tokens_.front().sourceRange().end();
			tokens_.pop_front();
			if (tokens_.empty()) {
				tokens_.push_back(source_.lexToken());
			}
			position_ = tokens_.front().sourceRange().start();
			assert(!tokens_.empty());
		}
		
		bool TokenReader::consumeIfPresent(const Token::Kind tokenKind) {
			if (peek().kind() == tokenKind) {
				consume();
				return true;
			} else {
				return false;
			}
		}
		
		void TokenReader::expect(const Token::Kind tokenKind) {
			(void) expectOneOf({ tokenKind });
		}
		
		static bool isIn(const Token::Kind findKind, const Array<Token::Kind, 4>& tokenKinds) {
			bool found = false;
			for (const auto kind: tokenKinds) {
				if (findKind == kind) {
					found = true;
					break;
				}
			}
			return found;
		}
		
		Token TokenReader::expectOneOf(const Array<Token::Kind, 4>& tokenKinds) {
			assert(!tokens_.empty());
			assert(!tokenKinds.empty());
			
			const auto token = peek();
			if (isIn(token.kind(), tokenKinds)) {
				consume();
			} else {
				issueDiag(UnexpectedTokenDiag(tokenKinds.copy(), token.kind()), position());
				
				for (size_t offset = 0; offset < 5; offset++) {
					const auto lookaheadToken = peek(offset);
					
					if (lookaheadToken.kind() == Token::END) {
						break;
					}
					
					if (isIn(lookaheadToken.kind(), tokenKinds)) {
						for (size_t i = 0; i < (offset + 1); i++) {
							consume();
						}
						break;
					}
				}
			}
			return token;
		}
		
		String TokenReader::expectName() {
			const auto token = expectOneOf({ Token::NAME });
			if (token.kind() == Token::NAME) {
				return token.name();
			} else {
				return makeCString("<none>");
			}
		}
		
		Version TokenReader::expectVersion() {
			Version version;
			if (peek().kind() == Token::VERSION) {
				version = peek().version();
			} else {
				throw std::logic_error("TODO: version expected");
			}
			expect(Token::VERSION);
			return version;
		}
		
		Debug::SourcePosition TokenReader::position() const {
			return position_;
		}
		
		Debug::SourcePosition TokenReader::lastTokenEndPosition() const {
			return lastEndPosition_;
		}
		
		Debug::SourceRange TokenReader::rangeFrom(const Debug::SourcePosition start) const {
			if (start <= lastTokenEndPosition()) {
				return Debug::SourceRange(start, lastTokenEndPosition());
			} else {
				return Debug::SourceRange(start, position());
			}
		}
		
		Debug::SourceLocation
		TokenReader::locationWithRangeFrom(const Debug::SourcePosition start) const {
			const auto range = rangeFrom(start);
			return Debug::SourceLocation(source_.fileName(), range);
		}
		
	}
	
}