#include "gtest/gtest.h"

#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/VarParser.hpp>
#include <locic/Support/StringHost.hpp>

#include "MockTokenSource.hpp"

namespace locic {
	
	namespace Parser {
		
		template <typename FnType>
		void testParseVar(const Array<Token::Kind, 16>& tokenKinds, FnType fn) {
			StringHost stringHost;
			MockTokenSource tokenSource(stringHost, tokenKinds);
			TokenReader tokenReader(tokenSource, tokenSource);
			const auto var = VarParser(tokenReader).parseVar();
			EXPECT_TRUE(tokenSource.allConsumed());
			EXPECT_TRUE(tokenReader.peek().kind() == Token::END);
			fn(var);
		}
		
		TEST(VarParseTest, TypeVarNamedType) {
			auto tokens = {
				Token::NAME,
				Token::NAME
			};
			testParseVar(tokens, [](const AST::Node<AST::TypeVar>& var) {
				EXPECT_TRUE(var->isNamed());
				EXPECT_TRUE(var->namedType()->isObjectType());
			});
		}
		
		TEST(VarParseTest, TypeVarConstType) {
			auto tokens = {
				Token::CONST,
				Token::NAME,
				Token::NAME
			};
			testParseVar(tokens, [](const AST::Node<AST::TypeVar>& var) {
				EXPECT_TRUE(var->isNamed());
				EXPECT_TRUE(var->namedType()->isConst());
				EXPECT_TRUE(var->namedType()->getConstTarget()->isObjectType());
			});
		}
		
		TEST(VarParseTest, TypeVarPointerType) {
			auto tokens = {
				Token::NAME,
				Token::STAR,
				Token::NAME
			};
			testParseVar(tokens, [](const AST::Node<AST::TypeVar>& var) {
				EXPECT_TRUE(var->isNamed());
				EXPECT_TRUE(var->namedType()->isPointer());
				EXPECT_TRUE(var->namedType()->getPointerTarget()->isObjectType());
			});
		}
		
		TEST(VarParseTest, TypeVarReferenceType) {
			auto tokens = {
				Token::NAME,
				Token::AMPERSAND,
				Token::NAME
			};
			testParseVar(tokens, [](const AST::Node<AST::TypeVar>& var) {
				EXPECT_TRUE(var->isNamed());
				EXPECT_TRUE(var->namedType()->isReference());
				EXPECT_TRUE(var->namedType()->getReferenceTarget()->isObjectType());
			});
		}
		
		TEST(VarParseTest, TypeVarEmptyPatternType) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET
			};
			testParseVar(tokens, [](const AST::Node<AST::TypeVar>& var) {
				EXPECT_TRUE(var->isPattern());
				EXPECT_TRUE(var->patternType()->isObjectType());
				EXPECT_EQ(var->typeVarList()->size(), 0);
			});
		}
		
		TEST(VarParseTest, TypeVarNestedEmptyPatternType) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::RROUNDBRACKET
			};
			testParseVar(tokens, [](const AST::Node<AST::TypeVar>& var) {
				EXPECT_TRUE(var->isPattern());
				EXPECT_TRUE(var->patternType()->isObjectType());
				EXPECT_EQ(var->typeVarList()->size(), 1);
				EXPECT_TRUE(var->typeVarList()->at(0)->isPattern());
				EXPECT_TRUE(var->typeVarList()->at(0)->patternType()->isObjectType());
				EXPECT_EQ(var->typeVarList()->at(0)->typeVarList()->size(), 0);
			});
		}
		
		TEST(VarParseTest, TypeVarPatternTypeWithAny) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::UNDERSCORE,
				Token::RROUNDBRACKET
			};
			testParseVar(tokens, [](const AST::Node<AST::TypeVar>& var) {
				EXPECT_TRUE(var->isPattern());
				EXPECT_TRUE(var->patternType()->isObjectType());
				EXPECT_EQ(var->typeVarList()->size(), 1);
				EXPECT_TRUE(var->typeVarList()->at(0)->isAny());
			});
		}
		
		TEST(VarParseTest, TypeVarPatternTypeWithMultipleAny) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::UNDERSCORE,
				Token::COMMA,
				Token::UNDERSCORE,
				Token::RROUNDBRACKET
			};
			testParseVar(tokens, [](const AST::Node<AST::TypeVar>& var) {
				EXPECT_TRUE(var->isPattern());
				EXPECT_TRUE(var->patternType()->isObjectType());
				EXPECT_EQ(var->typeVarList()->size(), 2);
				EXPECT_TRUE(var->typeVarList()->at(0)->isAny());
				EXPECT_TRUE(var->typeVarList()->at(1)->isAny());
			});
		}
		
		TEST(VarParseTest, TypeVarPatternTypeWithNamedType) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseVar(tokens, [](const AST::Node<AST::TypeVar>& var) {
				EXPECT_TRUE(var->isPattern());
				EXPECT_TRUE(var->patternType()->isObjectType());
				EXPECT_EQ(var->typeVarList()->size(), 1);
				EXPECT_TRUE(var->typeVarList()->at(0)->isNamed());
			});
		}
		
		TEST(VarParseTest, TypeVarPatternTypeWithMultipleNamedType) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::NAME,
				Token::COMMA,
				Token::NAME,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseVar(tokens, [](const AST::Node<AST::TypeVar>& var) {
				EXPECT_TRUE(var->isPattern());
				EXPECT_TRUE(var->patternType()->isObjectType());
				EXPECT_EQ(var->typeVarList()->size(), 2);
				EXPECT_TRUE(var->typeVarList()->at(0)->isNamed());
				EXPECT_TRUE(var->typeVarList()->at(1)->isNamed());
			});
		}
		
		TEST(VarParseTest, TypeVarPatternTypeWithNamedTypeAndAny) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::NAME,
				Token::COMMA,
				Token::UNDERSCORE,
				Token::RROUNDBRACKET
			};
			testParseVar(tokens, [](const AST::Node<AST::TypeVar>& var) {
				EXPECT_TRUE(var->isPattern());
				EXPECT_TRUE(var->patternType()->isObjectType());
				EXPECT_EQ(var->typeVarList()->size(), 2);
				EXPECT_TRUE(var->typeVarList()->at(0)->isNamed());
				EXPECT_TRUE(var->typeVarList()->at(1)->isAny());
			});
		}
		
	}
	
}
