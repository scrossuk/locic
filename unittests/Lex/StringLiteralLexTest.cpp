#include "gtest/gtest.h"

#include <locic/Lex/Lexer.hpp>
#include <locic/Lex/Token.hpp>
#include <locic/Support/StringHost.hpp>

#include "LexTest.hpp"
#include "MockCharacterSource.hpp"
#include "MockDiagnosticReceiver.hpp"

void testStringLiteral(const std::string& literal, const std::string& result,
                       const size_t endLineNumber = 1, size_t endColumn = 0) {
	if (endColumn == 0) {
		endColumn = literal.size() + 1;
	}
	
	const auto start = locic::Debug::SourcePosition(/*lineNumber=*/1, /*column=*/1,
	                                         /*byteOffset=*/0);
	const auto end = locic::Debug::SourcePosition(endLineNumber, endColumn,
	                                       /*byteOffset=*/literal.size());
	const auto range = locic::Debug::SourceRange(start, end);
	locic::StringHost stringHost;
	const auto stringResult = locic::String(stringHost, result);
	testLexer(literal, { locic::Lex::Token::Constant(locic::Constant::StringVal(stringResult), range) },
	          /*diags=*/{}, &stringHost);
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
	testStringLiteral("\"\n\"", "\n", 2, 2);
}

void testValidEscape(const char c, const char escapeChar) {
	testStringLiteral(std::string("\"\\") + c + "\"", std::string() + escapeChar);
}

void testValidEscapeString(const std::string& string, const char escapeChar) {
	testStringLiteral(std::string("\"\\") + string + "\"", std::string() + escapeChar);
}

TEST(StringLiteralLexTest, LegalEscapeCharacter) {
	const auto validEscapes = { 'a', 'b', 'f', 'n', 'r', 't', 'v', '\\', '"', '\'' };
	const auto escapeValues = { '\a', '\b', '\f', '\n', '\r', '\t', '\v', '\\', '"', '\'' };
	for (auto i = validEscapes.begin(), j = escapeValues.begin();
	     i != validEscapes.end(); ++i, ++j) {
		testValidEscape(*i, *j);
	}
}

TEST(StringLiteralLexTest, OctalEscapeCharacter) {
	for (unsigned char i = 0; i < 128; i++) {
		const auto valueString = locic::makeString("%o", (unsigned) i);
		testValidEscapeString(valueString, i);
	}
}

void testStringLiteralError(const std::string& literal, const std::string& result, const std::initializer_list<locic::Lex::DiagID> diags) {
	const auto start = locic::Debug::SourcePosition(/*lineNumber=*/1, /*column=*/1,
	                                                /*byteOffset=*/0);
	const auto end = locic::Debug::SourcePosition(/*lineNumber=*/1,
	                                              /*column=*/literal.size() + 1,
	                                              /*byteOffset=*/literal.size());
	const auto range = locic::Debug::SourceRange(start, end);
	
	locic::StringHost stringHost;
	const auto stringResult = locic::String(stringHost, result);
	testLexer(literal, { locic::Lex::Token::Constant(locic::Constant::StringVal(stringResult), range) },
	          diags, &stringHost);
}

TEST(StringLiteralLexTest, UnterminatedStringLiteral) {
	testStringLiteralError("\"", "", { locic::Lex::DiagID::UnterminatedStringLiteral });
	testStringLiteralError("\"\\", "\\", { locic::Lex::DiagID::InvalidStringLiteralEscape,
	                       locic::Lex::DiagID::UnterminatedStringLiteral });
	testStringLiteralError("\"\\\\", "\\", { locic::Lex::DiagID::UnterminatedStringLiteral });
	testStringLiteralError("\"\\\"", "\"", { locic::Lex::DiagID::UnterminatedStringLiteral });
}

void testInvalidEscape(const char c) {
	testStringLiteralError(std::string("\"\\") + c + "\"", std::string("\\") + c,
			       { locic::Lex::DiagID::InvalidStringLiteralEscape });
}

TEST(StringLiteralLexTest, IllegalEscapeCharacter) {
	const auto invalidEscapes = { 'c', 'd', 'e', 'g', 'h', 'i', 'j', 'k',
		'l', 'm', 'o', 'p', 'q', 's', 'u', 'w', 'x', 'y', 'z' };
	for (const char c: invalidEscapes) {
		testInvalidEscape(c);
	}
}
