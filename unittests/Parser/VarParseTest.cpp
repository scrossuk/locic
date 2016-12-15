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
			const auto var = VarParser(tokenReader).parseVar(/*allowInherit=*/true);
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
				EXPECT_TRUE(var->type()->isObjectType());
			});
		}
		
		TEST(VarParseTest, DISABLED_VarInheritNamedType) {
			auto tokens = {
				Token::INHERIT,
				Token::NAME,
				Token::NAME
			};
			testParseVar(tokens, [](const AST::Node<AST::Var>& var) {
				// TODO: check for inherited property
				EXPECT_TRUE(var->isNamed());
				EXPECT_TRUE(var->type()->isObjectType());
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
				EXPECT_TRUE(var->type()->isConst());
				EXPECT_TRUE(var->type()->getConstTarget()->isObjectType());
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
				EXPECT_TRUE(var->type()->isPointer());
				EXPECT_TRUE(var->type()->getPointerTarget()->isObjectType());
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
				EXPECT_TRUE(var->type()->isReference());
				EXPECT_TRUE(var->type()->getReferenceTarget()->isObjectType());
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
				EXPECT_TRUE(var->type()->isObjectType());
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
				EXPECT_TRUE(var->type()->isObjectType());
				EXPECT_EQ(var->varList()->size(), 1);
				EXPECT_TRUE(var->varList()->at(0)->isPattern());
				EXPECT_TRUE(var->varList()->at(0)->type()->isObjectType());
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
				EXPECT_TRUE(var->type()->isObjectType());
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
				EXPECT_TRUE(var->type()->isObjectType());
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
				EXPECT_TRUE(var->type()->isObjectType());
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
				EXPECT_TRUE(var->type()->isObjectType());
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
				EXPECT_TRUE(var->type()->isObjectType());
				EXPECT_EQ(var->varList()->size(), 2);
				EXPECT_TRUE(var->varList()->at(0)->isNamed());
				EXPECT_TRUE(var->varList()->at(1)->isAny());
			});
		}
		
	}
	
}
