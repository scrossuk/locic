#ifndef MOCKTOKENSOURCE_HPP
#define MOCKTOKENSOURCE_HPP

#include <stdexcept>

#include <locic/Constant.hpp>
#include <locic/Debug/SourceLocation.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Debug/SourceRange.hpp>
#include <locic/Lex/LexerAPI.hpp>
#include <locic/Parser/DiagnosticReceiver.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/StringHost.hpp>

#include "TokenGenerator.hpp"

namespace locic {
	
	namespace Parser {
		
		class MockTokenSource: public Lex::LexerAPI, public Parser::DiagnosticReceiver {
		public:
			MockTokenSource(const StringHost& stringHost,
			                const Array<Token::Kind, 16>& tokenKinds)
			: stringHost_(stringHost), tokenKinds_(tokenKinds),
			position_(0), tokenGenerator_(stringHost),
			sourcePosition_(0, 0, 0) { }
			
			Token lexToken() {
				auto token = lexTokenWithoutLocationInfo();
				
				const Debug::SourcePosition endPosition(
				    sourcePosition_.lineNumber(),
				    sourcePosition_.column() + 1,
				    sourcePosition_.byteOffset() + 1
				);
				
				token.setSourceRange(Debug::SourceRange(sourcePosition_,
				                                        endPosition));
				
				sourcePosition_ = Debug::SourcePosition(
				    sourcePosition_.lineNumber(),
				    sourcePosition_.column() + 2,
				    sourcePosition_.byteOffset() + 2
				);
				
				return token;
			}
			
			Token lexTokenWithoutLocationInfo() {
				if (position_ == tokenKinds_.size()) {
					return Token::Basic(Token::END);
				} else {
					return tokenGenerator_.makeToken(tokenKinds_[position_++]);
				}
			}
			
			String fileName() const {
				return String(stringHost_, "<test file>");
			}
			
			bool allConsumed() const {
				return position_ == tokenKinds_.size();
			}
			
			void issueDiag(std::unique_ptr<Parser::Diag> /*diag*/,
			               const Debug::SourceLocation& /*location*/) {
				throw std::logic_error("Unexpected parser error.");
			}
			
		private:
			const StringHost& stringHost_;
			const Array<Token::Kind, 16>& tokenKinds_;
			size_t position_;
			TokenGenerator tokenGenerator_;
			Debug::SourcePosition sourcePosition_;
			
		};
		
	}
	
}

#endif
