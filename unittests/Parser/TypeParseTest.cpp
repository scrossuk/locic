#include "gtest/gtest.h"

#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeParser.hpp>
#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/StringHost.hpp>

#include "MockTokenSource.hpp"

namespace locic {
	
	namespace Parser {
		
		template <typename FnType>
		void testParseType(const Array<Token::Kind, 16>& tokenKinds, FnType fn) {
			StringHost stringHost;
			MockTokenSource tokenSource(stringHost, tokenKinds);
			TokenReader tokenReader(tokenSource);
			const auto type = TypeParser(tokenReader).parseType();
			EXPECT_TRUE(tokenSource.allConsumed());
			EXPECT_TRUE(tokenReader.peek().kind() == Token::END);
			fn(type);
		}
		
		void testPrimitiveType(const Array<Token::Kind, 16>& tokenKinds, const PrimitiveID primitiveID) {
			testParseType(tokenKinds, [=](const AST::Node<AST::Type>& type) {
				ASSERT_TRUE(type->isPrimitive());
				EXPECT_EQ(type->primitiveID(), primitiveID);
			});
		}
		
		TEST(TypeParseTest, CoreTypes) {
			testPrimitiveType({ Token::VOID }, PrimitiveVoid);
			testPrimitiveType({ Token::BOOL }, PrimitiveBool);
			testPrimitiveType({ Token::TYPENAME }, PrimitiveTypename);
		}
		
		TEST(TypeParseTest, IntegerTypes) {
			testPrimitiveType({ Token::BYTE }, PrimitiveByte);
			testPrimitiveType({ Token::SHORT }, PrimitiveShort);
			testPrimitiveType({ Token::SIGNED, Token::SHORT }, PrimitiveShort);
			testPrimitiveType({ Token::SHORT, Token::INT }, PrimitiveShort);
			testPrimitiveType({ Token::SIGNED, Token::SHORT, Token::INT }, PrimitiveShort);
			testPrimitiveType({ Token::INT }, PrimitiveInt);
			testPrimitiveType({ Token::SIGNED }, PrimitiveInt);
			testPrimitiveType({ Token::SIGNED, Token::INT }, PrimitiveInt);
			testPrimitiveType({ Token::LONG }, PrimitiveLong);
			testPrimitiveType({ Token::LONG, Token::INT }, PrimitiveLong);
			testPrimitiveType({ Token::LONG, Token::LONG }, PrimitiveLongLong);
			testPrimitiveType({ Token::SIGNED, Token::LONG, Token::LONG }, PrimitiveLongLong);
			testPrimitiveType({ Token::LONG, Token::LONG, Token::INT }, PrimitiveLongLong);
			testPrimitiveType({ Token::SIGNED, Token::LONG, Token::LONG, Token::INT }, PrimitiveLongLong);
			testPrimitiveType({ Token::LONGLONG }, PrimitiveLongLong);
			testPrimitiveType({ Token::SIGNED, Token::LONGLONG }, PrimitiveLongLong);
			
			testPrimitiveType({ Token::UNSIGNED, Token::BYTE }, PrimitiveUByte);
			testPrimitiveType({ Token::UBYTE }, PrimitiveUByte);
			testPrimitiveType({ Token::UNSIGNED, Token::SHORT }, PrimitiveUShort);
			testPrimitiveType({ Token::UNSIGNED, Token::SHORT, Token::INT }, PrimitiveUShort);
			testPrimitiveType({ Token::USHORT}, PrimitiveUShort);
			testPrimitiveType({ Token::UNSIGNED, Token::INT }, PrimitiveUInt);
			testPrimitiveType({ Token::UNSIGNED }, PrimitiveUInt);
			testPrimitiveType({ Token::UINT }, PrimitiveUInt);
			testPrimitiveType({ Token::UNSIGNED, Token::LONG }, PrimitiveULong);
			testPrimitiveType({ Token::UNSIGNED, Token::LONG, Token::INT }, PrimitiveULong);
			testPrimitiveType({ Token::ULONG }, PrimitiveULong);
			testPrimitiveType({ Token::UNSIGNED, Token::LONG, Token::LONG }, PrimitiveULongLong);
			testPrimitiveType({ Token::UNSIGNED, Token::LONG, Token::LONG, Token::INT }, PrimitiveULongLong);
			testPrimitiveType({ Token::UNSIGNED, Token::LONGLONG }, PrimitiveULongLong);
			testPrimitiveType({ Token::ULONGLONG }, PrimitiveULongLong);
		}
		
		TEST(TypeParseTest, FloatTypes) {
			testPrimitiveType({ Token::FLOAT }, PrimitiveFloat);
			testPrimitiveType({ Token::DOUBLE }, PrimitiveDouble);
			testPrimitiveType({ Token::LONG, Token::DOUBLE }, PrimitiveLongDouble);
		}
		
		TEST(TypeParseTest, SymbolOneComponent) {
			testParseType({ Token::NAME }, [](const AST::Node<AST::Type>& type) {
				EXPECT_TRUE(type->isObjectType());
				EXPECT_EQ(type->objectType.symbol->size(), 1);
				EXPECT_EQ(type->objectType.symbol->at(0)->templateArguments()->size(), 0);
			});
		}
		
		TEST(TypeParseTest, SymbolTwoComponents) {
			auto tokens = {
				Token::NAME,
				Token::DOUBLE_COLON,
				Token::NAME
			};
			testParseType(tokens, [](const AST::Node<AST::Type>& type) {
				EXPECT_TRUE(type->isObjectType());
				EXPECT_EQ(type->objectType.symbol->size(), 2);
				EXPECT_EQ(type->objectType.symbol->at(0)->templateArguments()->size(), 0);
				EXPECT_EQ(type->objectType.symbol->at(1)->templateArguments()->size(), 0);
			});
		}
		
		TEST(TypeParseTest, SymbolThreeComponents) {
			auto tokens = {
				Token::NAME,
				Token::DOUBLE_COLON,
				Token::NAME,
				Token::DOUBLE_COLON,
				Token::NAME
			};
			testParseType(tokens, [](const AST::Node<AST::Type>& type) {
				EXPECT_TRUE(type->isObjectType());
				EXPECT_EQ(type->objectType.symbol->size(), 3);
				EXPECT_EQ(type->objectType.symbol->at(0)->templateArguments()->size(), 0);
				EXPECT_EQ(type->objectType.symbol->at(1)->templateArguments()->size(), 0);
				EXPECT_EQ(type->objectType.symbol->at(2)->templateArguments()->size(), 0);
			});
		}
		
		TEST(TypeParseTest, SymbolTemplated) {
			auto tokens = {
				Token::NAME,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::RTRIBRACKET
			};
			testParseType(tokens, [](const AST::Node<AST::Type>& type) {
				EXPECT_TRUE(type->isObjectType());
				EXPECT_EQ(type->objectType.symbol->size(), 1);
				EXPECT_EQ(type->objectType.symbol->at(0)->templateArguments()->size(), 1);
			});
		}
		
		TEST(TypeParseTest, SymbolTemplatedChild) {
			auto tokens = {
				Token::NAME,
				Token::DOUBLE_COLON,
				Token::NAME,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::RTRIBRACKET
			};
			testParseType(tokens, [](const AST::Node<AST::Type>& type) {
				EXPECT_TRUE(type->isObjectType());
				EXPECT_EQ(type->objectType.symbol->size(), 2);
				EXPECT_EQ(type->objectType.symbol->at(0)->templateArguments()->size(), 0);
				EXPECT_EQ(type->objectType.symbol->at(1)->templateArguments()->size(), 1);
			});
		}
		
		TEST(TypeParseTest, SymbolTemplatedParent) {
			auto tokens = {
				Token::NAME,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::DOUBLE_COLON,
				Token::NAME
			};
			testParseType(tokens, [](const AST::Node<AST::Type>& type) {
				EXPECT_TRUE(type->isObjectType());
				EXPECT_EQ(type->objectType.symbol->size(), 2);
				EXPECT_EQ(type->objectType.symbol->at(0)->templateArguments()->size(), 1);
				EXPECT_EQ(type->objectType.symbol->at(1)->templateArguments()->size(), 0);
			});
		}
		
		TEST(TypeParseTest, ConstType) {
			auto tokens = {
				Token::CONST,
				Token::NAME
			};
			testParseType(tokens, [](const AST::Node<AST::Type>& type) {
				EXPECT_TRUE(type->isConst());
				EXPECT_TRUE(type->getConstTarget()->isObjectType());
			});
		}
		
		TEST(TypeParseTest, ConstTrivialTruePredicateType) {
			auto tokens = {
				Token::CONST,
				Token::LTRIBRACKET,
				Token::TRUEVAL,
				Token::RTRIBRACKET,
				Token::NAME
			};
			testParseType(tokens, [](const AST::Node<AST::Type>& type) {
				ASSERT_TRUE(type->isConstPredicate());
				EXPECT_EQ(type->getConstPredicate()->kind(), AST::Predicate::TRUE);
				EXPECT_TRUE(type->getConstPredicateTarget()->isObjectType());
			});
		}
		
		TEST(TypeParseTest, ConstTrivialFalsePredicateType) {
			auto tokens = {
				Token::CONST,
				Token::LTRIBRACKET,
				Token::FALSEVAL,
				Token::RTRIBRACKET,
				Token::NAME
			};
			testParseType(tokens, [](const AST::Node<AST::Type>& type) {
				ASSERT_TRUE(type->isConstPredicate());
				EXPECT_EQ(type->getConstPredicate()->kind(), AST::Predicate::FALSE);
				EXPECT_TRUE(type->getConstPredicateTarget()->isObjectType());
			});
		}
		
		TEST(TypeParseTest, ConstAndPredicateType) {
			auto tokens = {
				Token::CONST,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::AND,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::NAME
			};
			testParseType(tokens, [](const AST::Node<AST::Type>& type) {
				ASSERT_TRUE(type->isConstPredicate());
				EXPECT_EQ(type->getConstPredicate()->kind(), AST::Predicate::AND);
				EXPECT_TRUE(type->getConstPredicateTarget()->isObjectType());
			});
		}
		
		TEST(TypeParseTest, PointerType) {
			auto tokens = {
				Token::NAME,
				Token::STAR
			};
			testParseType(tokens, [](const AST::Node<AST::Type>& type) {
				EXPECT_TRUE(type->isPointer());
				EXPECT_TRUE(type->getPointerTarget()->isObjectType());
			});
		}
		
		TEST(TypeParseTest, ConstPointerType) {
			auto tokens = {
				Token::CONST,
				Token::NAME,
				Token::STAR
			};
			testParseType(tokens, [](const AST::Node<AST::Type>& type) {
				EXPECT_TRUE(type->isPointer());
				EXPECT_TRUE(type->getPointerTarget()->isConst());
				EXPECT_TRUE(type->getPointerTarget()->getConstTarget()->isObjectType());
			});
		}
		
		TEST(TypeParseTest, ReferenceType) {
			auto tokens = {
				Token::NAME,
				Token::AMPERSAND
			};
			testParseType(tokens, [](const AST::Node<AST::Type>& type) {
				EXPECT_TRUE(type->isReference());
				EXPECT_TRUE(type->getReferenceTarget()->isObjectType());
			});
		}
		
		TEST(TypeParseTest, ConstReferenceType) {
			auto tokens = {
				Token::CONST,
				Token::NAME,
				Token::AMPERSAND
			};
			testParseType(tokens, [](const AST::Node<AST::Type>& type) {
				EXPECT_TRUE(type->isReference());
				EXPECT_TRUE(type->getReferenceTarget()->isConst());
				EXPECT_TRUE(type->getReferenceTarget()->getConstTarget()->isObjectType());
			});
		}
		
		TEST(TypeParseTest, StaticArrayType) {
			auto tokens = {
				Token::NAME,
				Token::LSQUAREBRACKET,
				Token::NAME,
				Token::RSQUAREBRACKET
			};
			testParseType(tokens, [](const AST::Node<AST::Type>& type) {
				ASSERT_TRUE(type->isStaticArray());
				EXPECT_TRUE(type->getStaticArrayTarget()->isObjectType());
				EXPECT_TRUE(type->getArraySize()->isSymbol());
			});
		}
		
		TEST(TypeParseTest, ChainedStaticArrayType) {
			auto tokens = {
				Token::NAME,
				Token::LSQUAREBRACKET,
				Token::NAME,
				Token::RSQUAREBRACKET,
				Token::LSQUAREBRACKET,
				Token::NAME,
				Token::RSQUAREBRACKET
			};
			testParseType(tokens, [](const AST::Node<AST::Type>& type) {
				ASSERT_TRUE(type->isStaticArray());
				ASSERT_TRUE(type->getStaticArrayTarget()->isStaticArray());
				EXPECT_TRUE(type->getStaticArrayTarget()->getStaticArrayTarget()->isObjectType());
				EXPECT_TRUE(type->getStaticArrayTarget()->getArraySize()->isSymbol());
				EXPECT_TRUE(type->getArraySize()->isSymbol());
			});
		}
		
	}
	
}
