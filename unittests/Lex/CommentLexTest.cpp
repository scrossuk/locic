#include "gtest/gtest.h"

#include <locic/Lex/Lexer.hpp>
#include <locic/Lex/Token.hpp>
#include <locic/Support/StringHost.hpp>

#include "LexTest.hpp"
#include "MockCharacterSource.hpp"
#include "MockDiagnosticReceiver.hpp"

void testComment(const std::string& text, const size_t lineNumber, const size_t column) {
	// Add a '0' so there's a token to read.
	const auto textWithToken = text + "1";
	const auto start = locic::Debug::SourcePosition(lineNumber, column,
	                                                /*byteOffset=*/text.size());
	const auto end = locic::Debug::SourcePosition(lineNumber, column + 1,
	                                              /*byteOffset=*/text.size() + 1);
	const auto range = locic::Debug::SourceRange(start, end);
	testLexer(textWithToken, { locic::Lex::Token::Constant(locic::Constant::Integer(1), range) }, /*diags=*/{});
}

TEST(CommentLexTest, ShortComment) {
	testComment("//\n", /*lineNumber=*/2, /*column=*/1);
	testComment("//text\n", /*lineNumber=*/2, /*column=*/1);
}

TEST(CommentLexTest, NestedShortComment) {
	testComment("///\n", /*lineNumber=*/2, /*column=*/1);
	testComment("////\n", /*lineNumber=*/2, /*column=*/1);
	testComment("/////\n", /*lineNumber=*/2, /*column=*/1);
	testComment("//////\n", /*lineNumber=*/2, /*column=*/1);
	testComment("//text\n", /*lineNumber=*/2, /*column=*/1);
	testComment("///text\n", /*lineNumber=*/2, /*column=*/1);
	testComment("////text\n", /*lineNumber=*/2, /*column=*/1);
	testComment("/////text\n", /*lineNumber=*/2, /*column=*/1);
	testComment("//////text\n", /*lineNumber=*/2, /*column=*/1);
}

TEST(CommentLexTest, SideBySideShortComments) {
	testComment("//\n//\n", /*lineNumber=*/3, /*column=*/1);
}

TEST(CommentLexTest, SpacesInShortComment) {
	testComment("// \n", /*lineNumber=*/2, /*column=*/1);
	testComment("//\t\n", /*lineNumber=*/2, /*column=*/1);
}

TEST(CommentLexTest, EmptyLongComment) {
	testComment("/**/", /*lineNumber=*/1, /*column=*/5);
}

TEST(CommentLexTest, TrivialLongComment) {
	testComment("/*text*/", /*lineNumber=*/1, /*column=*/9);
}

TEST(CommentLexTest, NewlineInLongComment) {
	testComment("/*\n*/", /*lineNumber=*/2, /*column=*/3);
	testComment("/*\n\n*/", /*lineNumber=*/3, /*column=*/3);
	testComment("/*\n\n\n*/", /*lineNumber=*/4, /*column=*/3);
}

TEST(CommentLexTest, SideBySideLongComments) {
	testComment("/**//**/", /*lineNumber=*/1, /*column=*/9);
}

TEST(CommentLexTest, SpacesInLongComment) {
	testComment("/* */", /*lineNumber=*/1, /*column=*/6);
	testComment("/*\t*/", /*lineNumber=*/1, /*column=*/6);
}
