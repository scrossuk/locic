#ifndef LOCIC_PARSER_LEXLEXER_HPP
#define LOCIC_PARSER_LEXLEXER_HPP

#include <cstdio>

#include <locic/Frontend/DiagnosticReceiver.hpp>
#include <locic/Lex/Character.hpp>
#include <locic/Lex/CharacterSource.hpp>
#include <locic/Lex/FileCharacterSource.hpp>
#include <locic/Lex/Lexer.hpp>
#include <locic/Lex/Token.hpp>

namespace locic {
	
	namespace Parser {
		
		class LexLexer {
		public:
			LexLexer(FILE * file, const String fileName,
			         DiagnosticReceiver& diagReceiver)
			: source_(fileName, file), lexer_(source_, diagReceiver) { }
			
			~LexLexer() { }
			
			LexLexer(const LexLexer&) = delete;
			LexLexer& operator=(const LexLexer&) = delete;
			
			LexLexer(LexLexer&&) = delete;
			LexLexer& operator=(LexLexer&&) = delete;
			
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
