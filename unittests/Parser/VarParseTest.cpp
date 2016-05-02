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
		
		TEST(VarParseTest, VarNamedType) {
			auto tokens = {
				Token::NAME,
				Token::NAME
			};
			testParseVar(tokens, [](const AST::Node<AST::Var>& var) {
				EXPECT_TRUE(var->isNamed());
				EXPECT_TRUE(var->namedType()->isObjectType());
			});
		}
		
		TEST(VarParseTest, VarConstType) {
			auto tokens = {
				Token::CONST,
				Token::NAME,
				Token::NAME
			};
			testParseVar(tokens, [](const AST::Node<AST::Var>& var) {
				EXPECT_TRUE(var->isNamed());
				EXPECT_TRUE(var->namedType()->isConst());
				EXPECT_TRUE(var->namedType()->getConstTarget()->isObjectType());
			});
		}
		
		TEST(VarParseTest, VarPointerType) {
			auto tokens = {
				Token::NAME,
				Token::STAR,
				Token::NAME
			};
			testParseVar(tokens, [](const AST::Node<AST::Var>& var) {
				EXPECT_TRUE(var->isNamed());
				EXPECT_TRUE(var->namedType()->isPointer());
				EXPECT_TRUE(var->namedType()->getPointerTarget()->isObjectType());
			});
		}
		
		TEST(VarParseTest, VarReferenceType) {
			auto tokens = {
				Token::NAME,
				Token::AMPERSAND,
				Token::NAME
			};
			testParseVar(tokens, [](const AST::Node<AST::Var>& var) {
				EXPECT_TRUE(var->isNamed());
				EXPECT_TRUE(var->namedType()->isReference());
				EXPECT_TRUE(var->namedType()->getReferenceTarget()->isObjectType());
			});
		}
		
		TEST(VarParseTest, VarEmptyPatternType) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET
			};
			testParseVar(tokens, [](const AST::Node<AST::Var>& var) {
				EXPECT_TRUE(var->isPattern());
				EXPECT_TRUE(var->patternType()->isObjectType());
				EXPECT_EQ(var->varList()->size(), 0);
			});
		}
		
		TEST(VarParseTest, VarNestedEmptyPatternType) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::RROUNDBRACKET
			};
			testParseVar(tokens, [](const AST::Node<AST::Var>& var) {
				EXPECT_TRUE(var->isPattern());
				EXPECT_TRUE(var->patternType()->isObjectType());
				EXPECT_EQ(var->varList()->size(), 1);
				EXPECT_TRUE(var->varList()->at(0)->isPattern());
				EXPECT_TRUE(var->varList()->at(0)->patternType()->isObjectType());
				EXPECT_EQ(var->varList()->at(0)->varList()->size(), 0);
			});
		}
		
		TEST(VarParseTest, VarPatternTypeWithAny) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::UNDERSCORE,
				Token::RROUNDBRACKET
			};
			testParseVar(tokens, [](const AST::Node<AST::Var>& var) {
				EXPECT_TRUE(var->isPattern());
				EXPECT_TRUE(var->patternType()->isObjectType());
				EXPECT_EQ(var->varList()->size(), 1);
				EXPECT_TRUE(var->varList()->at(0)->isAny());
			});
		}
		
		TEST(VarParseTest, VarPatternTypeWithMultipleAny) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::UNDERSCORE,
				Token::COMMA,
				Token::UNDERSCORE,
				Token::RROUNDBRACKET
			};
			testParseVar(tokens, [](const AST::Node<AST::Var>& var) {
				EXPECT_TRUE(var->isPattern());
				EXPECT_TRUE(var->patternType()->isObjectType());
				EXPECT_EQ(var->varList()->size(), 2);
				EXPECT_TRUE(var->varList()->at(0)->isAny());
				EXPECT_TRUE(var->varList()->at(1)->isAny());
			});
		}
		
		TEST(VarParseTest, VarPatternTypeWithNamedType) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::NAME,
				Token::RROUNDBRACKET
			};
			testParseVar(tokens, [](const AST::Node<AST::Var>& var) {
				EXPECT_TRUE(var->isPattern());
				EXPECT_TRUE(var->patternType()->isObjectType());
				EXPECT_EQ(var->varList()->size(), 1);
				EXPECT_TRUE(var->varList()->at(0)->isNamed());
			});
		}
		
		TEST(VarParseTest, VarPatternTypeWithMultipleNamedType) {
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
			testParseVar(tokens, [](const AST::Node<AST::Var>& var) {
				EXPECT_TRUE(var->isPattern());
				EXPECT_TRUE(var->patternType()->isObjectType());
				EXPECT_EQ(var->varList()->size(), 2);
				EXPECT_TRUE(var->varList()->at(0)->isNamed());
				EXPECT_TRUE(var->varList()->at(1)->isNamed());
			});
		}
		
		TEST(VarParseTest, VarPatternTypeWithNamedTypeAndAny) {
			auto tokens = {
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::NAME,
				Token::COMMA,
				Token::UNDERSCORE,
				Token::RROUNDBRACKET
			};
			testParseVar(tokens, [](const AST::Node<AST::Var>& var) {
				EXPECT_TRUE(var->isPattern());
				EXPECT_TRUE(var->patternType()->isObjectType());
				EXPECT_EQ(var->varList()->size(), 2);
				EXPECT_TRUE(var->varList()->at(0)->isNamed());
				EXPECT_TRUE(var->varList()->at(1)->isAny());
			});
		}
		
	}
	
}
