#include "gtest/gtest.h"

#include <locic/Lex/Lexer.hpp>
#include <locic/Lex/Token.hpp>
#include <locic/Support/StringHost.hpp>

#include "MockCharacterSource.hpp"
#include "MockDiagnosticReceiver.hpp"

void testStringLiteral(const std::string& literal, const std::string& result) {
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
	EXPECT_EQ(token.constant().kind(), locic::Constant::STRING);
	EXPECT_EQ(result, token.constant().stringValue().asStdString());
}

TEST(StringLiteralLexTest, EmptyString) {
	testStringLiteral("\"\"", "");
}

TEST(StringLiteralLexTest, TrivialString) {
	testStringLiteral("\"thisistext\"", "thisistext");
}

TEST(StringLiteralLexTest, RawTab) {
	testStringLiteral("\"\t\"", "\t");
}

TEST(StringLiteralLexTest, RawNewline) {
	testStringLiteral("\"\n\"", "\n");
}

void testValidEscape(const char c, const char escapeChar) {
	testStringLiteral(std::string("\"\\") + c + "\"", std::string() + escapeChar);
}

void testValidEscapeString(const std::string& string, const char escapeChar) {
	testStringLiteral(std::string("\"\\") + string + "\"", std::string() + escapeChar);
}

TEST(StringLiteralLexTest, LegalEscapeCharacter) {
	const auto validEscapes = { 'a', 'b', 'f', 'n', 'r', 't', 'v', '\\', '"' };
	const auto escapeValues = { '\a', '\b', '\f', '\n', '\r', '\t', '\v', '\\', '"' };
	for (auto i = validEscapes.begin(), j = escapeValues.begin();
	     i != validEscapes.end(); ++i, ++j) {
		testValidEscape(*i, *j);
	}
}

TEST(StringLiteralLexTest, DISABLED_OctalEscapeCharacter) {
	for (unsigned char i = 0; i < 128; i++) {
		const auto valueString = locic::makeString("%o", (unsigned) i);
		testValidEscapeString(valueString, i);
	}
}

void testStringLiteralError(const std::string& literal, const std::string& result, const locic::Lex::Diag diag) {
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
	EXPECT_EQ(diagnosticReceiver.numErrors(), 1);
	EXPECT_EQ(diagnosticReceiver.getError(0), diag);
	EXPECT_EQ(token.kind(), locic::Lex::Token::CONSTANT);
	EXPECT_EQ(token.constant().kind(), locic::Constant::STRING);
	EXPECT_EQ(result, token.constant().stringValue().asStdString());
}

TEST(StringLiteralLexTest, UnterminatedStringLiteral) {
	testStringLiteralError("\"", "", locic::Lex::Diag::UnterminatedStringLiteral);
	testStringLiteralError("\"\\", "\\", locic::Lex::Diag::UnterminatedStringLiteral);
	testStringLiteralError("\"\\\\", "\\", locic::Lex::Diag::UnterminatedStringLiteral);
	testStringLiteralError("\"\\\"", "\"", locic::Lex::Diag::UnterminatedStringLiteral);
}

void testInvalidEscape(const char c) {
	testStringLiteralError(std::string("\"\\") + c + "\"", std::string("\\") + c,
	                       locic::Lex::Diag::InvalidStringLiteralEscape);
}

TEST(StringLiteralLexTest, IllegalEscapeCharacter) {
	const auto invalidEscapes = { 'c', 'd', 'e', 'g', 'h', 'i', 'j', 'k',
		'l', 'm', 'o', 'p', 'q', 's', 'u', 'w', 'x', 'y', 'z' };
	for (const char c: invalidEscapes) {
		testInvalidEscape(c);
	}
}
