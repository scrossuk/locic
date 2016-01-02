#ifndef TOKENGENERATOR_HPP
#define TOKENGENERATOR_HPP

#include <locic/Constant.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/StringHost.hpp>

namespace locic {
	
	namespace Parser {
		
		class TokenGenerator {
		public:
			TokenGenerator(const StringHost& stringHost)
			: stringHost_(stringHost), nextInteger_(0) { }
			
			Token makeToken(const Token::Kind kind) {
				switch (kind) {
					case Token::NAME:
						return Token::Name(String(stringHost_, "test"));
					case Token::CONSTANT:
						return Token::Constant(Constant::Integer(nextInteger_++));
					case Token::VERSION:
						return Token::Version(Version(1, 0, 0));
					default:
						return Token::Basic(kind);
				}
			}
			
		private:
			const StringHost& stringHost_;
			unsigned long long nextInteger_;
			
		};
		
	}
	
}

#endif
