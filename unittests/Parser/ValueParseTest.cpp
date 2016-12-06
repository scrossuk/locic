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
			TokenReader tokenReader(tokenSource, tokenSource);
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
			testPrimitiveTypeValue({ Token::UNICHAR }, PrimitiveUnichar);
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
		
		TEST(ValueParseTest, Null) {
			testParseValue({ Token::NULLVAL }, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::LITERAL);
				EXPECT_EQ(*(value->literal.constant), Constant::Null());
			});
		}
		
		TEST(ValueParseTest, True) {
			testParseValue({ Token::TRUEVAL }, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::LITERAL);
				EXPECT_EQ(*(value->literal.constant), Constant::True());
			});
		}
		
		TEST(ValueParseTest, False) {
			testParseValue({ Token::FALSEVAL }, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::LITERAL);
				EXPECT_EQ(*(value->literal.constant), Constant::False());
			});
		}
		
		TEST(ValueParseTest, AlignOf) {
			auto tokens = {
				Token::ALIGNOF,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::ALIGNOF);
				EXPECT_TRUE(value->alignOf.type->isObjectType());
			});
		}
		
		TEST(ValueParseTest, SizeOf) {
			auto tokens = {
				Token::SIZEOF,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::SIZEOF);
				EXPECT_TRUE(value->sizeOf.type->isObjectType());
			});
		}
		
		TEST(ValueParseTest, SizeOfMultiply) {
			auto tokens = {
				Token::SIZEOF,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET,
				Token::STAR,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_MULTIPLY);
				const auto& leftOperand = value->binaryOp.leftOperand;
				ASSERT_EQ(leftOperand->kind(), AST::Value::SIZEOF);
				EXPECT_TRUE(leftOperand->sizeOf.type->isObjectType());
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
		
		TEST(ValueParseTest, Plus) {
			auto tokens = {
				Token::PLUS,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::UNARYOP);
				EXPECT_EQ(value->unaryOp.kind, AST::OP_PLUS);
			});
		}
		
		TEST(ValueParseTest, Minus) {
			auto tokens = {
				Token::MINUS,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::UNARYOP);
				EXPECT_EQ(value->unaryOp.kind, AST::OP_MINUS);
			});
		}
		
		TEST(ValueParseTest, Not) {
			auto tokens = {
				Token::EXCLAIMMARK,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::UNARYOP);
				EXPECT_EQ(value->unaryOp.kind, AST::OP_NOT);
			});
		}
		
		TEST(ValueParseTest, Address) {
			auto tokens = {
				Token::AMPERSAND,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::UNARYOP);
				EXPECT_EQ(value->unaryOp.kind, AST::OP_ADDRESS);
			});
		}
		
		TEST(ValueParseTest, Deref) {
			auto tokens = {
				Token::STAR,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::UNARYOP);
				EXPECT_EQ(value->unaryOp.kind, AST::OP_DEREF);
			});
		}
		
		TEST(ValueParseTest, Move) {
			auto tokens = {
				Token::MOVE,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::UNARYOP);
				EXPECT_EQ(value->unaryOp.kind, AST::OP_MOVE);
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
		
		TEST(ValueParseTest, FunctionPointerType) {
			auto tokens = {
				Token::LROUNDBRACKET,
				Token::STAR,
				Token::RROUNDBRACKET,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::TYPEREF);
				ASSERT_TRUE(value->typeRef.type->isFunction());
				EXPECT_TRUE(value->typeRef.type->functionReturnType()->isObjectType());
			});
		}
		
		TEST(ValueParseTest, LvalType) {
			auto tokens = {
				Token::LVAL,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::TYPEREF);
				EXPECT_TRUE(value->typeRef.type->isLval());
			});
		}
		
		TEST(ValueParseTest, RefType) {
			auto tokens = {
				Token::REF,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::TYPEREF);
				EXPECT_TRUE(value->typeRef.type->isRef());
			});
		}
		
		TEST(ValueParseTest, StaticRefType) {
			auto tokens = {
				Token::STATICREF,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::TYPEREF);
				EXPECT_TRUE(value->typeRef.type->isStaticRef());
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
		
		TEST(ValueParseTest, IndexingOrArrayType) {
			auto tokens = {
				Token::NAME,
				Token::LSQUAREBRACKET,
				Token::NAME,
				Token::RSQUAREBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::MERGE);
				
				ASSERT_EQ(value->merge.first->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->merge.first->binaryOp.kind, AST::OP_INDEX);
				
				ASSERT_EQ(value->merge.second->kind(), AST::Value::TYPEREF);
				ASSERT_TRUE(value->merge.second->typeRef.type->isStaticArray());
				EXPECT_TRUE(value->merge.second->typeRef.type->getStaticArrayTarget()->isObjectType());
			});
		}
		
		TEST(ValueParseTest, ChainedIndexingOrArrayType) {
			auto tokens = {
				Token::NAME,
				Token::LSQUAREBRACKET,
				Token::NAME,
				Token::RSQUAREBRACKET,
				Token::LSQUAREBRACKET,
				Token::NAME,
				Token::RSQUAREBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::MERGE);
				
				const auto& first = value->merge.first;
				ASSERT_EQ(first->kind(), AST::Value::BINARYOP);
				ASSERT_EQ(first->binaryOp.leftOperand->kind(), AST::Value::MERGE);
				ASSERT_EQ(first->binaryOp.leftOperand->merge.first->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(first->binaryOp.leftOperand->merge.first->binaryOp.kind, AST::OP_INDEX);
				ASSERT_EQ(first->binaryOp.leftOperand->merge.second->kind(), AST::Value::TYPEREF);
				EXPECT_TRUE(first->binaryOp.leftOperand->merge.second->typeRef.type->isStaticArray());
				
				const auto& second = value->merge.second;
				ASSERT_EQ(second->kind(), AST::Value::TYPEREF);
				ASSERT_TRUE(second->typeRef.type->isStaticArray());
				EXPECT_TRUE(second->typeRef.type->getStaticArrayTarget()->isStaticArray());
			});
		}
		
		TEST(ValueParseTest, IndexingCallResult) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::LSQUAREBRACKET,
				Token::NAME,
				Token::RSQUAREBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_INDEX);
				EXPECT_EQ(value->binaryOp.leftOperand->kind(), AST::Value::FUNCTIONCALL);
			});
		}
		
		TEST(ValueParseTest, IntArrayType) {
			auto tokens = {
				Token::INT,
				Token::LSQUAREBRACKET,
				Token::NAME,
				Token::RSQUAREBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::TYPEREF);
				ASSERT_TRUE(value->typeRef.type->isStaticArray());
				EXPECT_TRUE(value->typeRef.type->getStaticArrayTarget()->isPrimitive());
			});
		}
		
		TEST(ValueParseTest, PointerArrayType) {
			auto tokens = {
				Token::NAME,
				Token::STAR,
				Token::LSQUAREBRACKET,
				Token::NAME,
				Token::RSQUAREBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::TYPEREF);
				ASSERT_TRUE(value->typeRef.type->isStaticArray());
				EXPECT_TRUE(value->typeRef.type->getStaticArrayTarget()->isPointer());
			});
		}
		
		TEST(ValueParseTest, ReferenceArrayType) {
			auto tokens = {
				Token::NAME,
				Token::AMPERSAND,
				Token::LSQUAREBRACKET,
				Token::NAME,
				Token::RSQUAREBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::TYPEREF);
				ASSERT_TRUE(value->typeRef.type->isStaticArray());
				EXPECT_TRUE(value->typeRef.type->getStaticArrayTarget()->isReference());
			});
		}
		
		TEST(ValueParseTest, Ternary) {
			auto tokens = {
				Token::NAME,
				Token::QUESTIONMARK,
				Token::NAME,
				Token::COLON,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::TERNARY);
				EXPECT_TRUE(value->ternary.condition->isSymbol());
				EXPECT_TRUE(value->ternary.ifTrue->isSymbol());
				EXPECT_TRUE(value->ternary.ifFalse->isSymbol());
			});
		}
		
		TEST(ValueParseTest, CapabilityTest) {
			auto tokens = {
				Token::NAME,
				Token::COLON,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::CAPABILITYTEST);
				EXPECT_TRUE(value->capabilityTest.checkType->isObjectType());
				EXPECT_TRUE(value->capabilityTest.capabilityType->isObjectType());
			});
		}
		
		TEST(ValueParseTest, NoTagValue) {
			auto tokens = {
				Token::NOTAG,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::TYPEREF);
				EXPECT_TRUE(value->typeRef.type->isNoTag());
			});
		}
		
		TEST(ValueParseTest, ConstCastValue) {
			auto tokens = {
				Token::CONST_CAST,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::COMMA,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::CAST);
				EXPECT_EQ(value->cast.castKind, AST::Value::CAST_CONST);
			});
		}
		
		TEST(ValueParseTest, DynamicCastValue) {
			auto tokens = {
				Token::DYNAMIC_CAST,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::COMMA,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::CAST);
				EXPECT_EQ(value->cast.castKind, AST::Value::CAST_DYNAMIC);
			});
		}
		
		TEST(ValueParseTest, ReinterpretCastValue) {
			auto tokens = {
				Token::REINTERPRET_CAST,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::COMMA,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::CAST);
				EXPECT_EQ(value->cast.castKind, AST::Value::CAST_REINTERPRET);
			});
		}
		
		TEST(ValueParseTest, RefValue) {
			auto tokens = {
				Token::REF,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::REF);
			});
		}
		
		TEST(ValueParseTest, NoRefValue) {
			auto tokens = {
				Token::NOREF,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::NOREF);
			});
		}
		
		TEST(ValueParseTest, LvalValue) {
			auto tokens = {
				Token::LVAL,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::LVAL);
			});
		}
		
		TEST(ValueParseTest, NoLvalValue) {
			auto tokens = {
				Token::NOLVAL,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::NOLVAL);
			});
		}
		
		TEST(ValueParseTest, AndTwoComparisonsNotTemplate) {
			auto tokens = {
				Token::NAME,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::DOUBLE_AMPERSAND,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_LOGICALAND);
				const auto& leftOperand = value->binaryOp.leftOperand;
				ASSERT_EQ(leftOperand->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(leftOperand->binaryOp.kind, AST::OP_LESSTHAN);
			});
		}
		
		TEST(ValueParseTest, OrTwoComparisonsNotTemplate) {
			auto tokens = {
				Token::NAME,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::DOUBLE_VERTICAL_BAR,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				ASSERT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_LOGICALOR);
				const auto& leftOperand = value->binaryOp.leftOperand;
				ASSERT_EQ(leftOperand->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(leftOperand->binaryOp.kind, AST::OP_LESSTHAN);
			});
		}
		
	}
	
}
