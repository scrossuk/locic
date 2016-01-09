#ifndef LOCIC_PARSER_LEXLEXER_HPP
#define LOCIC_PARSER_LEXLEXER_HPP

#include <cstdio>

#include <locic/Lex/Character.hpp>
#include <locic/Lex/CharacterSource.hpp>
#include <locic/Lex/DiagnosticReceiver.hpp>
#include <locic/Lex/FileCharacterSource.hpp>
#include <locic/Lex/Lexer.hpp>
#include <locic/Lex/Token.hpp>
#include <locic/Parser/Context.hpp>

namespace locic {
	
	namespace Parser {
		
		class LexLexer: public Lex::DiagnosticReceiver {
		public:
			LexLexer(FILE * file, const String fileName)
			: source_(fileName, file), lexer_(source_, *this) { }
			
			~LexLexer() { }
			
			LexLexer(const LexLexer&) = delete;
			LexLexer& operator=(const LexLexer&) = delete;
			
			LexLexer(LexLexer&&) = delete;
			LexLexer& operator=(LexLexer&&) = delete;
			
			void issueWarning(Lex::Diag /*kind*/, Debug::SourceRange range) {
				printf("Warning at %s\n", range.toString().c_str());
			}
			
			void issueError(Lex::Diag /*kind*/, Debug::SourceRange range) {
				printf("Error at %s\n", range.toString().c_str());
			}
			
			Lex::Lexer& getLexer() {
				return lexer_;
			}
			
		private:
			Lex::FileCharacterSource source_;
			Lex::Lexer lexer_;
			
		};
		
	}
	
}

#endif
