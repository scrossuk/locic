#ifndef LEXTEST_HPP
#define LEXTEST_HPP

#include <locic/Constant.hpp>
#include <locic/Lex/Character.hpp>
#include <locic/Lex/CharacterSource.hpp>
#include <locic/Lex/Lexer.hpp>
#include <locic/Support/Array.hpp>

#include "MockCharacterSource.hpp"
#include "MockDiagnosticReceiver.hpp"

namespace {

void testLexer(const std::string& input, const locic::Array<locic::Lex::Token, 2>& expectedTokens,
               const locic::Array<locic::Lex::Diag, 2>& expectedErrors, locic::StringHost* const stringHostPtr = nullptr) {
	locic::Array<locic::Lex::Character, 16> characters;
	for (const auto c: input) {
		characters.push_back(c);
	}
	
	locic::StringHost stringHost;
	locic::StringHost& useStringHost = stringHostPtr != nullptr ? *stringHostPtr : stringHost;
	MockCharacterSource source(std::move(characters));
	MockDiagnosticReceiver diagnosticReceiver;
	locic::Lex::Lexer lexer(source, diagnosticReceiver);
	
	locic::Array<locic::Lex::Token, 2> tokens;
	
	while (true) {
		const auto token = lexer.lexToken(useStringHost);
		if (token) {
			tokens.push_back(*token);
		} else {
			break;
		}
	}
	
	EXPECT_TRUE(source.empty());
	EXPECT_EQ(diagnosticReceiver.numErrors(), expectedErrors.size());
	
	for (size_t i = 0; i < std::min(diagnosticReceiver.numErrors(), expectedErrors.size()); i++) {
		EXPECT_EQ(diagnosticReceiver.getError(i), expectedErrors[i]);
	}
	
	EXPECT_EQ(tokens.size(), expectedTokens.size());
	
	for (size_t i = 0; i < std::min(tokens.size(), expectedTokens.size()); i++) {
		assert(!expectedTokens[i].sourceRange().isNull());
		EXPECT_EQ(tokens[i], expectedTokens[i]);
	}
}

}

#endif