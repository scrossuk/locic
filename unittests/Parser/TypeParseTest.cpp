#include "gtest/gtest.h"

#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeParser.hpp>
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
			fn(type);
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
		
	}
	
}
