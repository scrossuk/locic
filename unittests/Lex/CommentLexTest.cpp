#include "gtest/gtest.h"

#include <locic/Lex/Lexer.hpp>
#include <locic/Lex/Token.hpp>
#include <locic/Support/StringHost.hpp>

#include "MockCharacterSource.hpp"
#include "MockDiagnosticReceiver.hpp"

void testComment(const std::string& text) {
	// Add a '0' so there's a token to read.
	const auto textWithToken = text + "1";
	
	locic::Array<locic::Lex::Character, 16> characters;
	for (const auto c: textWithToken) {
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
	EXPECT_EQ(token.constant().kind(), locic::Constant::INTEGER);
	EXPECT_EQ(1, token.constant().integerValue());
}

TEST(CommentLexTest, ShortComment) {
	testComment("//\n");
	testComment("//text\n");
}

TEST(CommentLexTest, NestedShortComment) {
	testComment("///\n");
	testComment("////\n");
	testComment("/////\n");
	testComment("//////\n");
	testComment("//text\n");
	testComment("///text\n");
	testComment("////text\n");
	testComment("/////text\n");
	testComment("//////text\n");
}

TEST(CommentLexTest, SideBySideShortComments) {
	testComment("//\n//\n");
}

TEST(CommentLexTest, SpacesInShortComment) {
	testComment("// \n");
	testComment("//\t\n");
}

TEST(CommentLexTest, EmptyLongComment) {
	testComment("/**/");
}

TEST(CommentLexTest, TrivialLongComment) {
	testComment("/*text*/");
}

TEST(CommentLexTest, NewlineInLongComment) {
	testComment("/*\n*/");
	testComment("/*\n\n*/");
	testComment("/*\n\n\n*/");
}

TEST(CommentLexTest, SideBySideLongComments) {
	testComment("/**//**/");
}

TEST(CommentLexTest, SpacesInLongComment) {
	testComment("/* */");
	testComment("/*\t*/");
}
