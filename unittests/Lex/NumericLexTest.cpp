#include "gtest/gtest.h"

#include <locic/Lex/Lexer.hpp>
#include <locic/Lex/NumericValue.hpp>

#include "MockCharacterSource.hpp"
#include "MockDiagnosticReceiver.hpp"

locic::Lex::NumericValue parseNumericValueWithNoDiagnostics(locic::Array<locic::Lex::Character, 16> characters) {
	MockCharacterSource source(characters.copy());
	MockDiagnosticReceiver diagnosticReceiver;
	locic::Lex::Lexer lexer(source, diagnosticReceiver);
	const auto value = lexer.lexNumericConstant();
	EXPECT_TRUE(source.empty());
	EXPECT_TRUE(diagnosticReceiver.hasNoErrorsOrWarnings());
	return value;
}

TEST(NumericLexTest, Integer) {
	const auto value = parseNumericValueWithNoDiagnostics({'1', '2', '3'});
	EXPECT_EQ(value.kind(), locic::Lex::NumericValue::INTEGER);
	EXPECT_EQ(value.integerValue(), 123);
}

TEST(NumericLexTest, HexInteger) {
	const auto value = parseNumericValueWithNoDiagnostics({'0', 'x', 'A'});
	EXPECT_EQ(value.kind(), locic::Lex::NumericValue::INTEGER);
	EXPECT_EQ(value.integerValue(), 10);
}

TEST(NumericLexTest, Float) {
	const auto value = parseNumericValueWithNoDiagnostics({'1', '2', '3', '.', '2'});
	EXPECT_EQ(value.kind(), locic::Lex::NumericValue::FLOAT);
	EXPECT_EQ(value.floatValue(), 123.2);
}

TEST(NumericLexTest, FloatEmptyIntegerPart) {
	const auto value = parseNumericValueWithNoDiagnostics({'.', '5'});
	EXPECT_EQ(value.kind(), locic::Lex::NumericValue::FLOAT);
	EXPECT_EQ(value.floatValue(), 0.5);
}

TEST(NumericLexTest, FloatEmptyFractionalPart) {
	const auto value = parseNumericValueWithNoDiagnostics({'2', '0', '.'});
	EXPECT_EQ(value.kind(), locic::Lex::NumericValue::FLOAT);
	EXPECT_EQ(value.floatValue(), 20.0);
}
