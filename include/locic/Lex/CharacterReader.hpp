#ifndef LOCIC_LEX_CHARACTERREADER_HPP
#define LOCIC_LEX_CHARACTERREADER_HPP

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Frontend/DiagnosticReceiver.hpp>
#include <locic/Lex/Character.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Debug {
		
		class SourceRange;
		
	}
	
	namespace Lex {
		
		class CharacterSource;
		
		class CharacterReader {
		public:
			CharacterReader(CharacterSource& source, DiagnosticReceiver& diagReceiver);
			
			template <typename DiagType>
			void issueDiag(DiagType diag, const Debug::SourcePosition& start) {
				diagReceiver_.issueDiag(std::unique_ptr<DiagAPI>(new DiagType(std::move(diag))),
				                        locationWithRangeFrom(start));
			}
			
			template <typename DiagType>
			void issueDiagWithLoc(DiagType diag, const Debug::SourceLocation& location) {
				diagReceiver_.issueDiag(std::unique_ptr<DiagAPI>(new DiagType(std::move(diag))),
				                        location);
			}
			
			const StringHost& stringHost() const;
			
			CharacterSource& source();
			const CharacterSource& source() const;
			
			bool isEnd() const;
			
			Character get();
			
			Character peek();
			
			void consume();
			
			void expect(Character character);
			
			Debug::SourcePosition position() const;
			
			Debug::SourceRange rangeFrom(Debug::SourcePosition start) const;
			
			Debug::SourceLocation locationWithRangeFrom(Debug::SourcePosition start) const;
			
		private:
			const StringHost& stringHost_;
			CharacterSource& source_;
			DiagnosticReceiver& diagReceiver_;
			Character currentCharacter_;
			Debug::SourcePosition position_;
			
		};
		
	}
	
}

#endif
