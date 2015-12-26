#include "gtest/gtest.h"

#include <locic/Lex/Lexer.hpp>
#include <locic/Lex/Token.hpp>

#include "LexTest.hpp"

void testWhitespace(const std::string& text, const size_t lineNumber, const size_t column) {
	// Add a '0' so there's a token to read.
	const auto textWithToken = text + "1";
	const auto start = locic::Debug::SourcePosition(lineNumber, column,
	                                                /*byteOffset=*/text.size());
	const auto end = locic::Debug::SourcePosition(lineNumber, column + 1,
	                                              /*byteOffset=*/text.size() + 1);
	const auto range = locic::Debug::SourceRange(start, end);
	testLexer(textWithToken, { locic::Lex::Token::Constant(locic::Constant::Integer(1), range) }, /*diags=*/{});
}

TEST(WhitespaceLexTest, Spaces) {
	testWhitespace(" ", /*lineNumber=*/1, /*column=*/2);
	testWhitespace("  ", /*lineNumber=*/1, /*column=*/3);
	testWhitespace("   ", /*lineNumber=*/1, /*column=*/4);
	testWhitespace("    ", /*lineNumber=*/1, /*column=*/5);
	testWhitespace("     ", /*lineNumber=*/1, /*column=*/6);
}

TEST(WhitespaceLexTest, Tabs) {
	testWhitespace("\t", /*lineNumber=*/1, /*column=*/2);
	testWhitespace("\t\t", /*lineNumber=*/1, /*column=*/3);
}

TEST(WhitespaceLexTest, SpacesAndTabs) {
	testWhitespace(" \t ", /*lineNumber=*/1, /*column=*/4);
	testWhitespace("\t \t", /*lineNumber=*/1, /*column=*/4);
}

TEST(WhitespaceLexTest, Newlines) {
	testWhitespace("\n", /*lineNumber=*/2, /*column=*/1);
	testWhitespace("\n\n", /*lineNumber=*/3, /*column=*/1);
	testWhitespace("\n\n\n", /*lineNumber=*/4, /*column=*/1);
	testWhitespace("\n\n\n\n", /*lineNumber=*/5, /*column=*/1);
	testWhitespace("\n\n\n\n\n", /*lineNumber=*/6, /*column=*/1);
}

TEST(WhitespaceLexTest, NewlinesAndSpaces) {
	testWhitespace("\n ", /*lineNumber=*/2, /*column=*/2);
	testWhitespace("\n \n ", /*lineNumber=*/3, /*column=*/2);
	testWhitespace("\n \n \n ", /*lineNumber=*/4, /*column=*/2);
	testWhitespace("\n \n \n \n ", /*lineNumber=*/5, /*column=*/2);
	testWhitespace("\n \n \n \n \n ", /*lineNumber=*/6, /*column=*/2);
}

TEST(WhitespaceLexTest, NewlinesAndTabs) {
	testWhitespace("\n\t", /*lineNumber=*/2, /*column=*/2);
	testWhitespace("\n\t\n\t", /*lineNumber=*/3, /*column=*/2);
	testWhitespace("\n\t\n\t\n\t", /*lineNumber=*/4, /*column=*/2);
	testWhitespace("\n\t\n\t\n\t\n\t", /*lineNumber=*/5, /*column=*/2);
	testWhitespace("\n\t\n\t\n\t\n\t\n\t", /*lineNumber=*/6, /*column=*/2);
}
