#ifndef MOCKTOKENSOURCE_HPP
#define MOCKTOKENSOURCE_HPP

#include <locic/Constant.hpp>
#include <locic/Parser/TokenSource.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/StringHost.hpp>

#include "TokenGenerator.hpp"

namespace locic {
	
	namespace Parser {
		
		class MockTokenSource: public TokenSource {
		public:
			MockTokenSource(const StringHost& stringHost,
			                const Array<Token::Kind, 16>& tokenKinds)
			: stringHost_(stringHost), tokenKinds_(tokenKinds),
			position_(0), tokenGenerator_(stringHost) { }
			
			Token get() {
				if (position_ == tokenKinds_.size()) {
					return Token::Basic(Token::END);
				} else {
					return tokenGenerator_.makeToken(tokenKinds_[position_++]);
				}
			}
			
			String fileName() {
				return String(stringHost_, "<test file>");
			}
			
			bool allConsumed() const {
				return position_ == tokenKinds_.size();
			}
			
		private:
			const StringHost& stringHost_;
			const Array<Token::Kind, 16>& tokenKinds_;
			size_t position_;
			TokenGenerator tokenGenerator_;
			
		};
		
	}
	
}

#endif
