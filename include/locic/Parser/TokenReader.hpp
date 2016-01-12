#ifndef LOCIC_PARSER_TOKENREADER_HPP
#define LOCIC_PARSER_TOKENREADER_HPP

#include <deque>
#include <memory>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Frontend/DiagnosticReceiver.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Support/Array.hpp>

namespace locic {
	
	class String;
	class Version;
	
	namespace Debug {
		
		class SourceLocation;
		class SourceRange;
		
	}
	
	namespace Lex {
		
		class LexerAPI;
		
	}
	
	namespace Parser {
		
		class TokenReader {
		public:
			TokenReader(Lex::LexerAPI& source, DiagnosticReceiver& diagReceiver);
			
			template <typename DiagType>
			void issueDiag(DiagType diag, const Debug::SourcePosition& start) {
				diagReceiver_.issueDiag(std::unique_ptr<Diag>(new DiagType(std::move(diag))),
				                        locationWithRangeFrom(start));
			}
			
			template <typename DiagType>
			void issueDiagWithLoc(DiagType diag, const Debug::SourceLocation& location) {
				diagReceiver_.issueDiag(std::unique_ptr<Diag>(new DiagType(std::move(diag))),
				                        location);
			}
			
			String makeCString(const char* string) const;
			
			String makeString(std::string string) const;
			
			bool isEnd() const;
			
			Token get();
			
			Token peek(size_t offset = 0);
			
			void consume();
			
			bool consumeIfPresent(Token::Kind tokenKind);
			
			void expect(Token::Kind tokenKind);
			
			Token expectOneOf(const Array<Token::Kind, 4>& tokenKinds);
			
			String expectName();
			
			Version expectVersion();
			
			Debug::SourcePosition position() const;
			
			Debug::SourcePosition lastTokenEndPosition() const;
			
			Debug::SourceRange rangeFrom(Debug::SourcePosition start) const;
			
			Debug::SourceLocation locationWithRangeFrom(Debug::SourcePosition start) const;
			
		private:
			Lex::LexerAPI& source_;
			DiagnosticReceiver& diagReceiver_;
			std::deque<Token> tokens_;
			Debug::SourcePosition position_;
			Debug::SourcePosition lastEndPosition_;
			
		};
		
	}
	
}

#endif