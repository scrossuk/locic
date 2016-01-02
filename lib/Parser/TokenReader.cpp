#include <deque>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Debug/SourceRange.hpp>
#include <locic/Lex/LexerAPI.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace Parser {
		
		TokenReader::TokenReader(Lex::LexerAPI& source)
		: source_(source), tokens_(1, source.lexToken()),
		position_(tokens_.front().sourceRange().start()),
		lastEndPosition_(position_) { }
		
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
		
		Token TokenReader::expectOneOf(const Array<Token::Kind, 4>& tokenKinds) {
			assert(!tokens_.empty());
			assert(!tokenKinds.empty());
			
			const auto token = peek();
			
			bool found = false;
			for (const auto kind: tokenKinds) {
				if (token.kind() == kind) {
					found = true;
					break;
				}
			}
			
			if (found) {
				consume();
				return token;
			} else {
				std::string expectStr = "{ ";
				for (size_t i = 0; i < tokenKinds.size(); i++) {
					expectStr += Token::kindToString(tokenKinds[i]);
					if ((i + 1) < tokenKinds.size()) {
						expectStr += ", ";
					}
				}
				expectStr += " }";
				
				printf("Expected one of %s; got %s\n", expectStr.c_str(),
				       token.toString().c_str());
				throw std::logic_error("TODO: expected token not found");
			}
		}
		
		String TokenReader::expectName() {
			String name;
			if (peek().kind() == Token::NAME) {
				name = peek().name();
			} else {
				throw std::logic_error("TODO: name expected");
			}
			expect(Token::NAME);
			return name;
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