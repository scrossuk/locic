#include "gtest/gtest.h"

#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/ValueParser.hpp>
#include <locic/Support/StringHost.hpp>

#include "MockTokenSource.hpp"

namespace locic {
	
	namespace Parser {
		
		template <typename FnType>
		void testParseValue(const Array<Token::Kind, 16>& tokenKinds, FnType fn) {
			StringHost stringHost;
			MockTokenSource tokenSource(stringHost, tokenKinds);
			TokenReader tokenReader(tokenSource);
			const auto value = ValueParser(tokenReader).parseValue();
			EXPECT_TRUE(tokenSource.allConsumed());
			EXPECT_TRUE(tokenReader.peek().kind() == Token::END);
			fn(value);
		}
		
		void testPrimitiveTypeValue(const Array<Token::Kind, 16>& tokenKinds, const PrimitiveID primitiveID) {
			testParseValue(tokenKinds, [=](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::TYPEREF);
				ASSERT_TRUE(value->typeRef.type->isPrimitive());
				EXPECT_EQ(value->typeRef.type->primitiveID(), primitiveID);
			});
		}
		
		TEST(ValueParseTest, CoreTypes) {
			testPrimitiveTypeValue({ Token::VOID }, PrimitiveVoid);
			testPrimitiveTypeValue({ Token::BOOL }, PrimitiveBool);
			testPrimitiveTypeValue({ Token::TYPENAME }, PrimitiveTypename);
		}
		
		TEST(ValueParseTest, IntegerTypes) {
			testPrimitiveTypeValue({ Token::BYTE }, PrimitiveByte);
			testPrimitiveTypeValue({ Token::SHORT }, PrimitiveShort);
			testPrimitiveTypeValue({ Token::SIGNED, Token::SHORT }, PrimitiveShort);
			testPrimitiveTypeValue({ Token::SHORT, Token::INT }, PrimitiveShort);
			testPrimitiveTypeValue({ Token::SIGNED, Token::SHORT, Token::INT }, PrimitiveShort);
			testPrimitiveTypeValue({ Token::INT }, PrimitiveInt);
			testPrimitiveTypeValue({ Token::SIGNED }, PrimitiveInt);
			testPrimitiveTypeValue({ Token::SIGNED, Token::INT }, PrimitiveInt);
			testPrimitiveTypeValue({ Token::LONG }, PrimitiveLong);
			testPrimitiveTypeValue({ Token::LONG, Token::INT }, PrimitiveLong);
			testPrimitiveTypeValue({ Token::LONG, Token::LONG }, PrimitiveLongLong);
			testPrimitiveTypeValue({ Token::SIGNED, Token::LONG, Token::LONG }, PrimitiveLongLong);
			testPrimitiveTypeValue({ Token::LONG, Token::LONG, Token::INT }, PrimitiveLongLong);
			testPrimitiveTypeValue({ Token::SIGNED, Token::LONG, Token::LONG, Token::INT }, PrimitiveLongLong);
			testPrimitiveTypeValue({ Token::LONGLONG }, PrimitiveLongLong);
			testPrimitiveTypeValue({ Token::SIGNED, Token::LONGLONG }, PrimitiveLongLong);
			
			testPrimitiveTypeValue({ Token::UNSIGNED, Token::BYTE }, PrimitiveUByte);
			testPrimitiveTypeValue({ Token::UBYTE }, PrimitiveUByte);
			testPrimitiveTypeValue({ Token::UNSIGNED, Token::SHORT }, PrimitiveUShort);
			testPrimitiveTypeValue({ Token::UNSIGNED, Token::SHORT, Token::INT }, PrimitiveUShort);
			testPrimitiveTypeValue({ Token::USHORT}, PrimitiveUShort);
			testPrimitiveTypeValue({ Token::UNSIGNED, Token::INT }, PrimitiveUInt);
			testPrimitiveTypeValue({ Token::UNSIGNED }, PrimitiveUInt);
			testPrimitiveTypeValue({ Token::UINT }, PrimitiveUInt);
			testPrimitiveTypeValue({ Token::UNSIGNED, Token::LONG }, PrimitiveULong);
			testPrimitiveTypeValue({ Token::UNSIGNED, Token::LONG, Token::INT }, PrimitiveULong);
			testPrimitiveTypeValue({ Token::ULONG }, PrimitiveULong);
			testPrimitiveTypeValue({ Token::UNSIGNED, Token::LONG, Token::LONG }, PrimitiveULongLong);
			testPrimitiveTypeValue({ Token::UNSIGNED, Token::LONG, Token::LONG, Token::INT }, PrimitiveULongLong);
			testPrimitiveTypeValue({ Token::UNSIGNED, Token::LONGLONG }, PrimitiveULongLong);
			testPrimitiveTypeValue({ Token::ULONGLONG }, PrimitiveULongLong);
		}
		
		TEST(ValueParseTest, FloatTypes) {
			testPrimitiveTypeValue({ Token::FLOAT }, PrimitiveFloat);
			testPrimitiveTypeValue({ Token::DOUBLE }, PrimitiveDouble);
			testPrimitiveTypeValue({ Token::LONG, Token::DOUBLE }, PrimitiveLongDouble);
		}
		
		TEST(ValueParseTest, Literal) {
			testParseValue({ Token::CONSTANT }, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::LITERAL);
			});
		}
		
		TEST(ValueParseTest, Self) {
			testParseValue({ Token::SELF }, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::SELF);
			});
		}
		
		TEST(ValueParseTest, This) {
			testParseValue({ Token::THIS }, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::THIS);
			});
		}
		
		TEST(ValueParseTest, InternalConstructNoArguments) {
			auto tokens = {
				Token::AT,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::INTERNALCONSTRUCT);
				EXPECT_EQ(value->internalConstruct.templateArgs->size(), 0);
				EXPECT_EQ(value->internalConstruct.parameters->size(), 0);
			});
		}
		
		TEST(ValueParseTest, InternalConstructOneArgument) {
			auto tokens = {
				Token::AT,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::INTERNALCONSTRUCT);
				EXPECT_EQ(value->internalConstruct.templateArgs->size(), 0);
				EXPECT_EQ(value->internalConstruct.parameters->size(), 1);
			});
		}
		
		TEST(ValueParseTest, InternalConstructTwoArguments) {
			auto tokens = {
				Token::AT,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::COMMA,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::INTERNALCONSTRUCT);
				EXPECT_EQ(value->internalConstruct.templateArgs->size(), 0);
				EXPECT_EQ(value->internalConstruct.parameters->size(), 2);
			});
		}
		
		TEST(ValueParseTest, TemplatedInternalConstructOneArgument) {
			auto tokens = {
				Token::AT,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::INTERNALCONSTRUCT);
				EXPECT_EQ(value->internalConstruct.templateArgs->size(), 1);
				EXPECT_EQ(value->internalConstruct.parameters->size(), 0);
			});
		}
		
		TEST(ValueParseTest, TemplatedInternalConstructTwoArguments) {
			auto tokens = {
				Token::AT,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::COMMA,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::INTERNALCONSTRUCT);
				EXPECT_EQ(value->internalConstruct.templateArgs->size(), 2);
				EXPECT_EQ(value->internalConstruct.parameters->size(), 0);
			});
		}
		
		TEST(ValueParseTest, PointerType) {
			auto tokens = {
				Token::NAME,
				Token::STAR
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::TYPEREF);
				ASSERT_TRUE(value->typeRef.type->isPointer());
				EXPECT_TRUE(value->typeRef.type->getPointerTarget()->isObjectType());
			});
		}
		
		TEST(ValueParseTest, ReferenceType) {
			auto tokens = {
				Token::NAME,
				Token::AMPERSAND
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::TYPEREF);
				ASSERT_TRUE(value->typeRef.type->isReference());
				EXPECT_TRUE(value->typeRef.type->getReferenceTarget()->isObjectType());
			});
		}
		
		TEST(ValueParseTest, LessThan) {
			auto tokens = {
				Token::NAME,
				Token::LTRIBRACKET,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_LESSTHAN);
				EXPECT_EQ(value->binaryOp.leftOperand->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->binaryOp.rightOperand->kind(), AST::Value::SYMBOLREF);
			});
		}
		
		TEST(ValueParseTest, GreaterThan) {
			auto tokens = {
				Token::NAME,
				Token::RTRIBRACKET,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_GREATERTHAN);
				EXPECT_EQ(value->binaryOp.leftOperand->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->binaryOp.rightOperand->kind(), AST::Value::SYMBOLREF);
			});
		}
		
		TEST(ValueParseTest, LeftShift) {
			auto tokens = {
				Token::NAME,
				Token::DOUBLE_LTRIBRACKET,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_LEFTSHIFT);
				EXPECT_EQ(value->binaryOp.leftOperand->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->binaryOp.rightOperand->kind(), AST::Value::SYMBOLREF);
			});
		}
		
		TEST(ValueParseTest, RightShift) {
			auto tokens = {
				Token::NAME,
				Token::RTRIBRACKET,
				Token::RTRIBRACKET,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_RIGHTSHIFT);
				EXPECT_EQ(value->binaryOp.leftOperand->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->binaryOp.rightOperand->kind(), AST::Value::SYMBOLREF);
			});
		}
		
		TEST(ValueParseTest, Add) {
			auto tokens = {
				Token::NAME,
				Token::PLUS,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_ADD);
			});
		}
		
		TEST(ValueParseTest, AddLeftAssociative) {
			auto tokens = {
				Token::NAME,
				Token::PLUS,
				Token::NAME,
				Token::PLUS,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_ADD);
				EXPECT_EQ(value->binaryOp.leftOperand->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.leftOperand->binaryOp.kind, AST::OP_ADD);
				EXPECT_EQ(value->binaryOp.rightOperand->kind(), AST::Value::SYMBOLREF);
			});
		}
		
		TEST(ValueParseTest, Multiply) {
			auto tokens = {
				Token::NAME,
				Token::STAR,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_MULTIPLY);
			});
		}
		
		TEST(ValueParseTest, MultiplyInAddLeft) {
			auto tokens = {
				Token::NAME,
				Token::STAR,
				Token::NAME,
				Token::PLUS,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_ADD);
				EXPECT_EQ(value->binaryOp.leftOperand->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.leftOperand->binaryOp.kind, AST::OP_MULTIPLY);
				EXPECT_EQ(value->binaryOp.rightOperand->kind(), AST::Value::SYMBOLREF);
			});
		}
		
		TEST(ValueParseTest, MultiplyInAddRight) {
			auto tokens = {
				Token::NAME,
				Token::PLUS,
				Token::NAME,
				Token::STAR,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_ADD);
				EXPECT_EQ(value->binaryOp.leftOperand->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->binaryOp.rightOperand->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.rightOperand->binaryOp.kind, AST::OP_MULTIPLY);
			});
		}
		
		TEST(ValueParseTest, SymbolTwoComponents) {
			auto tokens = {
				Token::NAME,
				Token::DOUBLE_COLON,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->symbolRef.symbol->size(), 2);
				EXPECT_EQ(value->symbolRef.symbol->at(0)->templateArguments()->size(), 0);
				EXPECT_EQ(value->symbolRef.symbol->at(1)->templateArguments()->size(), 0);
			});
		}
		
		TEST(ValueParseTest, SymbolThreeComponents) {
			auto tokens = {
				Token::NAME,
				Token::DOUBLE_COLON,
				Token::NAME,
				Token::DOUBLE_COLON,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->symbolRef.symbol->size(), 3);
				EXPECT_EQ(value->symbolRef.symbol->at(0)->templateArguments()->size(), 0);
				EXPECT_EQ(value->symbolRef.symbol->at(1)->templateArguments()->size(), 0);
				EXPECT_EQ(value->symbolRef.symbol->at(2)->templateArguments()->size(), 0);
			});
		}
		
		TEST(ValueParseTest, SymbolTemplatedChild) {
			auto tokens = {
				Token::NAME,
				Token::DOUBLE_COLON,
				Token::NAME,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::RTRIBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->symbolRef.symbol->size(), 2);
				EXPECT_EQ(value->symbolRef.symbol->at(0)->templateArguments()->size(), 0);
				EXPECT_EQ(value->symbolRef.symbol->at(1)->templateArguments()->size(), 1);
			});
		}
		
		TEST(ValueParseTest, SymbolTemplatedParent) {
			auto tokens = {
				Token::NAME,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::DOUBLE_COLON,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->symbolRef.symbol->size(), 2);
				EXPECT_EQ(value->symbolRef.symbol->at(0)->templateArguments()->size(), 1);
				EXPECT_EQ(value->symbolRef.symbol->at(1)->templateArguments()->size(), 0);
			});
		}
		
		TEST(ValueParseTest, CallLiteralMethod) {
			auto tokens = {
				Token::CONSTANT,
				Token::DOT,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::FUNCTIONCALL);
				ASSERT_EQ(value->functionCall.functionValue->kind(), AST::Value::MEMBERACCESS);
				const auto& object = value->functionCall.functionValue->memberAccess.object;
				EXPECT_TRUE(object->isLiteral());
			});
		}
		
		TEST(ValueParseTest, CallTemplatedSymbol) {
			auto tokens = {
				Token::NAME,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::FUNCTIONCALL);
				ASSERT_TRUE(value->functionCall.functionValue->isSymbol());
				const auto& symbol = value->functionCall.functionValue->symbol();
				ASSERT_EQ(symbol->size(), 1);
				EXPECT_EQ(symbol->at(0)->templateArguments()->size(), 1);
			});
		}
		
		TEST(ValueParseTest, DerefCallMethod) {
			auto tokens = {
				Token::NAME,
				Token::PTRACCESS,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::FUNCTIONCALL);
				ASSERT_EQ(value->functionCall.functionValue->kind(), AST::Value::MEMBERACCESS);
				const auto& object = value->functionCall.functionValue->memberAccess.object;
				ASSERT_EQ(object->kind(), AST::Value::UNARYOP);
				EXPECT_TRUE(object->unaryOp.operand->isSymbol());
			});
		}
		
		TEST(ValueParseTest, CallTwoComparisons) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::COMMA,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::FUNCTIONCALL);
				EXPECT_TRUE(value->functionCall.functionValue->isSymbol());
				const auto& parameters = value->functionCall.parameters;
				ASSERT_EQ(parameters->size(), 2);
				EXPECT_EQ(parameters->at(0)->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(parameters->at(1)->kind(), AST::Value::BINARYOP);
			});
		}
		
		TEST(ValueParseTest, ConstructTemplateInCall) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::COMMA,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::FUNCTIONCALL);
				EXPECT_TRUE(value->functionCall.functionValue->isSymbol());
				const auto& parameters = value->functionCall.parameters;
				ASSERT_EQ(parameters->size(), 1);
				EXPECT_EQ(parameters->at(0)->kind(), AST::Value::FUNCTIONCALL);
			});
		}
		
	}
	
}
