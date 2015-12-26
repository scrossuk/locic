#include "gtest/gtest.h"

#include <locic/Lex/Lexer.hpp>
#include <locic/Lex/Token.hpp>
#include <locic/Support/StringHost.hpp>

#include "MockCharacterSource.hpp"
#include "MockDiagnosticReceiver.hpp"

void testCharacterLiteral(const std::string& literal, const uint32_t result) {
	locic::Array<locic::Lex::Character, 16> characters;
	for (const auto c: literal) {
		characters.push_back(c);
	}
	
	locic::StringHost stringHost;
	MockCharacterSource source(std::move(characters));
	MockDiagnosticReceiver diagnosticReceiver;
	locic::Lex::Lexer lexer(source, diagnosticReceiver);
	const auto token = lexer.lexToken(stringHost);
	EXPECT_TRUE(source.empty());
	EXPECT_TRUE(diagnosticReceiver.hasNoErrorsOrWarnings());
	EXPECT_EQ(token.kind(), locic::Lex::Token::CONSTANT);
	EXPECT_EQ(token.constant().kind(), locic::Constant::CHARACTER);
	EXPECT_EQ(result, token.constant().characterValue());
}

TEST(CharacterLiteralLexTest, Alphabet) {
	testCharacterLiteral("'a'", 'a');
	testCharacterLiteral("'b'", 'b');
	testCharacterLiteral("'c'", 'c');
	testCharacterLiteral("'d'", 'd');
	testCharacterLiteral("'e'", 'e');
	testCharacterLiteral("'f'", 'f');
	testCharacterLiteral("'x'", 'x');
	testCharacterLiteral("'y'", 'y');
	testCharacterLiteral("'z'", 'z');
}

TEST(CharacterLiteralLexTest, Digit) {
	testCharacterLiteral("'0'", '0');
	testCharacterLiteral("'1'", '1');
	testCharacterLiteral("'2'", '2');
	testCharacterLiteral("'3'", '3');
	testCharacterLiteral("'4'", '4');
	testCharacterLiteral("'5'", '5');
	testCharacterLiteral("'6'", '6');
	testCharacterLiteral("'7'", '7');
	testCharacterLiteral("'8'", '8');
	testCharacterLiteral("'9'", '9');
}

namespace {

	void testValidEscape(const char c, const char escapeChar) {
		testCharacterLiteral(std::string("'\\") + c + "'", escapeChar);
	}
	
	void testValidEscapeString(const std::string& string, const char escapeChar) {
		testCharacterLiteral(std::string("'\\") + string + "'", escapeChar);
	}
	
}

TEST(CharacterLiteralLexTest, LegalEscapeCharacter) {
	const auto validEscapes = { 'a', 'b', 'f', 'n', 'r', 't', 'v', '\\', '"', '\'' };
	const auto escapeValues = { '\a', '\b', '\f', '\n', '\r', '\t', '\v', '\\', '"', '\'' };
	for (auto i = validEscapes.begin(), j = escapeValues.begin();
	     i != validEscapes.end(); ++i, ++j) {
		testValidEscape(*i, *j);
	}
}

TEST(CharacterLiteralLexTest, OctalEscapeCharacter) {
	for (unsigned char i = 0; i < 128; i++) {
		const auto valueString = locic::makeString("%o", (unsigned) i);
		testValidEscapeString(valueString, i);
	}
}

void testCharacterLiteralError(const std::string& literal, const uint32_t result, const std::initializer_list<locic::Lex::Diag> diags) {
	assert(diags.size() != 0);
	
	locic::Array<locic::Lex::Character, 16> characters;
	for (const auto c: literal) {
		characters.push_back(c);
	}
	
	locic::StringHost stringHost;
	MockCharacterSource source(std::move(characters));
	MockDiagnosticReceiver diagnosticReceiver;
	locic::Lex::Lexer lexer(source, diagnosticReceiver);
	const auto token = lexer.lexToken(stringHost);
	EXPECT_TRUE(source.empty());
	EXPECT_EQ(diagnosticReceiver.numErrors(), diags.size());
	
	size_t pos = 0;
	for (auto diag: diags) {
		EXPECT_EQ(diagnosticReceiver.getError(pos), diag);
		pos++;
	}
	EXPECT_EQ(token.kind(), locic::Lex::Token::CONSTANT);
	EXPECT_EQ(token.constant().kind(), locic::Constant::CHARACTER);
	EXPECT_EQ(result, token.constant().characterValue());
}

TEST(CharacterLiteralLexTest, EmptyCharacterLiteral) {
	testCharacterLiteralError("''", '\0', { locic::Lex::Diag::EmptyCharacterLiteral });
}

TEST(CharacterLiteralLexTest, MultiCharCharacterLiteral) {
	testCharacterLiteralError("'ab'", 'a', { locic::Lex::Diag::MultiCharCharacterLiteral });
	testCharacterLiteralError("'abc'", 'a', { locic::Lex::Diag::MultiCharCharacterLiteral });
}

TEST(CharacterLiteralLexTest, UnterminatedCharacterLiteral) {
	testCharacterLiteralError("'", '\0', { locic::Lex::Diag::UnterminatedCharacterLiteral });
	testCharacterLiteralError("'\\", '\\', { locic::Lex::Diag::InvalidStringLiteralEscape,
	                          locic::Lex::Diag::UnterminatedCharacterLiteral });
	testCharacterLiteralError("'\\\\", '\\', { locic::Lex::Diag::UnterminatedCharacterLiteral });
	testCharacterLiteralError("'\\'", '\'', { locic::Lex::Diag::UnterminatedCharacterLiteral });
}

namespace {
	
	void testInvalidEscape(const char c) {
		testCharacterLiteralError(std::string("'\\") + c + "'", '\\',
		                          { locic::Lex::Diag::InvalidStringLiteralEscape,
		                          locic::Lex::Diag::MultiCharCharacterLiteral });
	}
	
}

TEST(CharacterLiteralLexTest, IllegalEscapeCharacter) {
	const auto invalidEscapes = { 'c', 'd', 'e', 'g', 'h', 'i', 'j', 'k',
		'l', 'm', 'o', 'p', 'q', 's', 'u', 'w', 'x', 'y', 'z' };
	for (const char c: invalidEscapes) {
		testInvalidEscape(c);
	}
}


