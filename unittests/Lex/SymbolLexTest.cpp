#include "gtest/gtest.h"

#include <locic/Lex/Lexer.hpp>
#include <locic/Lex/Token.hpp>
#include <locic/Support/StringHost.hpp>

#include "MockCharacterSource.hpp"
#include "MockDiagnosticReceiver.hpp"

void testSymbol(const std::string& text, const locic::Lex::Token::Kind result) {
	locic::Array<locic::Lex::Character, 16> characters;
	for (const auto c: text) {
		characters.push_back(c);
	}
	
	locic::StringHost stringHost;
	MockCharacterSource source(std::move(characters));
	MockDiagnosticReceiver diagnosticReceiver;
	locic::Lex::Lexer lexer(source, diagnosticReceiver);
	const auto token = lexer.lexToken(stringHost);
	EXPECT_TRUE(source.empty());
	EXPECT_TRUE(diagnosticReceiver.hasNoErrorsOrWarnings());
	EXPECT_EQ(token.kind(), result);
}

TEST(SymbolLexTest, SingleCharacterSymbol) {
	testSymbol("_", locic::Lex::Token::Kind::UNDERSCORE);
	testSymbol("%", locic::Lex::Token::Kind::PERCENT);
	testSymbol("=", locic::Lex::Token::Kind::SETEQUAL);
	testSymbol("+", locic::Lex::Token::Kind::PLUS);
	testSymbol("-", locic::Lex::Token::Kind::MINUS);
	testSymbol("*", locic::Lex::Token::Kind::STAR);
	testSymbol("/", locic::Lex::Token::Kind::FORWARDSLASH);
	testSymbol("!", locic::Lex::Token::Kind::EXCLAIMMARK);
	testSymbol("&", locic::Lex::Token::Kind::AMPERSAND);
	testSymbol("|", locic::Lex::Token::Kind::VERTICAL_BAR);
	testSymbol("?", locic::Lex::Token::Kind::QUESTIONMARK);
	testSymbol("@", locic::Lex::Token::Kind::AT);
	testSymbol(",", locic::Lex::Token::Kind::COMMA);
	testSymbol(":", locic::Lex::Token::Kind::COLON);
	testSymbol(";", locic::Lex::Token::Kind::SEMICOLON);
	testSymbol("(", locic::Lex::Token::Kind::LROUNDBRACKET);
	testSymbol(")", locic::Lex::Token::Kind::RROUNDBRACKET);
	testSymbol("{", locic::Lex::Token::Kind::LCURLYBRACKET);
	testSymbol("}", locic::Lex::Token::Kind::RCURLYBRACKET);
	testSymbol("[", locic::Lex::Token::Kind::LSQUAREBRACKET);
	testSymbol("]", locic::Lex::Token::Kind::RSQUAREBRACKET);
	testSymbol(".", locic::Lex::Token::Kind::DOT);
	testSymbol("<", locic::Lex::Token::Kind::LTRIBRACKET);
	testSymbol(">", locic::Lex::Token::Kind::RTRIBRACKET);
	testSymbol("~", locic::Lex::Token::Kind::TILDA);
}

TEST(SymbolLexTest, DoubleCharacterSymbol) {
	testSymbol("==", locic::Lex::Token::Kind::ISEQUAL);
	testSymbol("!=", locic::Lex::Token::Kind::NOTEQUAL);
	testSymbol(">=", locic::Lex::Token::Kind::GREATEROREQUAL);
	testSymbol("<=", locic::Lex::Token::Kind::LESSOREQUAL);
	testSymbol("->", locic::Lex::Token::Kind::PTRACCESS);
	testSymbol("+=", locic::Lex::Token::Kind::ADDEQUAL);
	testSymbol("-=", locic::Lex::Token::Kind::SUBEQUAL);
	testSymbol("*=", locic::Lex::Token::Kind::MULEQUAL);
	testSymbol("/=", locic::Lex::Token::Kind::DIVEQUAL);
	testSymbol("%=", locic::Lex::Token::Kind::PERCENTEQUAL);
	testSymbol("++", locic::Lex::Token::Kind::DOUBLE_PLUS);
	testSymbol("--", locic::Lex::Token::Kind::DOUBLE_MINUS);
	testSymbol("&&", locic::Lex::Token::Kind::DOUBLE_AMPERSAND);
	testSymbol("||", locic::Lex::Token::Kind::DOUBLE_VERTICAL_BAR);
	testSymbol("::", locic::Lex::Token::Kind::DOUBLE_COLON);
	testSymbol("<<", locic::Lex::Token::Kind::DOUBLE_LTRIBRACKET);
}
