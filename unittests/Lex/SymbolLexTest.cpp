#include "gtest/gtest.h"

#include <locic/Lex/Lexer.hpp>
#include <locic/Lex/Token.hpp>
#include <locic/Support/StringHost.hpp>

#include "LexTest.hpp"
#include "MockCharacterSource.hpp"
#include "MockDiagnosticReceiver.hpp"

void testSymbol(const std::string& text, const locic::Lex::Token::Kind result) {
	const auto start = locic::Debug::SourcePosition(/*lineNumber=*/1, /*column=*/1,
	                                         /*byteOffset=*/0);
	const auto end = locic::Debug::SourcePosition(/*lineNumber=*/1, /*column=*/text.size() + 1,
	                                       /*byteOffset=*/text.size());
	const auto range = locic::Debug::SourceRange(start, end);
	testLexer(text, { locic::Lex::Token::Basic(result, range) }, /*diags=*/{});
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
