#include "gtest/gtest.h"

#include <locic/Parser/StatementParser.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Support/StringHost.hpp>

#include "MockTokenSource.hpp"

namespace locic {
	
	namespace Parser {
		
		template <typename FnType>
		void testParseStatement(const Array<Token::Kind, 16>& tokenKinds, FnType fn) {
			StringHost stringHost;
			MockTokenSource tokenSource(stringHost, tokenKinds);
			TokenReader tokenReader(tokenSource);
			const auto statement = StatementParser(tokenReader).parseStatement();
			EXPECT_TRUE(tokenSource.allConsumed());
			fn(statement);
		}
		
		TEST(StatementParseTest, AssignConstant) {
			auto tokens = {
				Token::NAME,
				Token::SETEQUAL,
				Token::CONSTANT
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isAssign());
				EXPECT_TRUE(statement->assignLvalue()->isSymbol());
				EXPECT_TRUE(statement->assignRvalue()->isLiteral());
			});
		}
		
		TEST(StatementParseTest, AddAssignConstant) {
			auto tokens = {
				Token::NAME,
				Token::ADDEQUAL,
				Token::CONSTANT
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isAssign());
				EXPECT_EQ(statement->assignKind(), AST::ASSIGN_ADD);
				EXPECT_TRUE(statement->assignLvalue()->isSymbol());
				EXPECT_TRUE(statement->assignRvalue()->isLiteral());
			});
		}
		
		TEST(StatementParseTest, SubtractAssignConstant) {
			auto tokens = {
				Token::NAME,
				Token::SUBEQUAL,
				Token::CONSTANT
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isAssign());
				EXPECT_EQ(statement->assignKind(), AST::ASSIGN_SUB);
				EXPECT_TRUE(statement->assignLvalue()->isSymbol());
				EXPECT_TRUE(statement->assignRvalue()->isLiteral());
			});
		}
		
		TEST(StatementParseTest, MultiplyAssignConstant) {
			auto tokens = {
				Token::NAME,
				Token::MULEQUAL,
				Token::CONSTANT
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isAssign());
				EXPECT_EQ(statement->assignKind(), AST::ASSIGN_MUL);
				EXPECT_TRUE(statement->assignLvalue()->isSymbol());
				EXPECT_TRUE(statement->assignRvalue()->isLiteral());
			});
		}
		
		TEST(StatementParseTest, DivideAssignConstant) {
			auto tokens = {
				Token::NAME,
				Token::DIVEQUAL,
				Token::CONSTANT
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isAssign());
				EXPECT_EQ(statement->assignKind(), AST::ASSIGN_DIV);
				EXPECT_TRUE(statement->assignLvalue()->isSymbol());
				EXPECT_TRUE(statement->assignRvalue()->isLiteral());
			});
		}
		
		TEST(StatementParseTest, VarDeclNamedTypeAssignConstant) {
			auto tokens = {
				Token::NAME,
				Token::NAME,
				Token::SETEQUAL,
				Token::CONSTANT
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isVarDecl());
				ASSERT_TRUE(statement->varDeclVar()->isNamed());
				EXPECT_TRUE(statement->varDeclVar()->namedType()->isObjectType());
				EXPECT_TRUE(statement->varDeclValue()->isLiteral());
			});
		}
		
		TEST(StatementParseTest, VarDeclTemplatedTypeAssignConstant) {
			auto tokens = {
				Token::NAME,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::NAME,
				Token::SETEQUAL,
				Token::CONSTANT
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isVarDecl());
				const auto& var = statement->varDeclVar();
				ASSERT_TRUE(var->isNamed());
				ASSERT_TRUE(var->namedType()->isObjectType());
				ASSERT_EQ(var->namedType()->symbol()->size(), 1);
				EXPECT_EQ(var->namedType()->symbol()->at(0)->templateArguments()->size(), 1);
				EXPECT_TRUE(statement->varDeclValue()->isLiteral());
			});
		}
		
		TEST(StatementParseTest, VarDeclPointerTypeAssignConstant) {
			auto tokens = {
				Token::NAME,
				Token::STAR,
				Token::NAME,
				Token::SETEQUAL,
				Token::CONSTANT
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isVarDecl());
				ASSERT_TRUE(statement->varDeclVar()->isNamed());
				EXPECT_TRUE(statement->varDeclVar()->namedType()->isPointer());
				EXPECT_TRUE(statement->varDeclValue()->isLiteral());
			});
		}
		
	}
	
}
