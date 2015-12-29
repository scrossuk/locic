#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TokenSource.hpp>

namespace locic {
	
	namespace Parser {
		
		TokenReader::TokenReader(TokenSource& source)
		: source_(source), currentToken_(source.get()),
		position_(currentToken_.sourceRange().start()),
		lastEndPosition_(position_) { }
		
		bool TokenReader::isEnd() const {
			return currentToken_.isEnd();
		}
		
		Token TokenReader::get() {
			assert(!isEnd());
			const auto currentToken = currentToken_;
			consume();
			return currentToken;
		}
		
		Token TokenReader::peek() {
			return currentToken_;
		}
		
		void TokenReader::consume() {
			assert(!isEnd());
			lastEndPosition_ = currentToken_.sourceRange().end();
			currentToken_ = source_.get();
			position_ = currentToken_.sourceRange().start();
		}
		
		void TokenReader::expect(const Token::Kind tokenKind) {
			assert(currentToken_.kind() == tokenKind);
			consume();
		}
		
		Debug::SourcePosition TokenReader::position() const {
			return position_;
		}
		
		Debug::SourcePosition TokenReader::lastTokenEndPosition() const {
			return lastEndPosition_;
		}
		
	}
	
}