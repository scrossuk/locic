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
			EXPECT_TRUE(tokenReader.peek().kind() == Token::END);
			fn(statement);
		}
		
		TEST(StatementParseTest, ValueName) {
			auto tokens = {
				Token::NAME,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isValue());
				EXPECT_TRUE(statement->value()->isSymbol());
			});
		}
		
		TEST(StatementParseTest, ValueConstant) {
			auto tokens = {
				Token::CONSTANT,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isValue());
				EXPECT_TRUE(statement->value()->isLiteral());
			});
		}
		
		TEST(StatementParseTest, Increment) {
			auto tokens = {
				Token::NAME,
				Token::DOUBLE_PLUS,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isIncrement());
				EXPECT_TRUE(statement->incrementValue()->isSymbol());
			});
		}
		
		TEST(StatementParseTest, Decrement) {
			auto tokens = {
				Token::NAME,
				Token::DOUBLE_MINUS,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isDecrement());
				EXPECT_TRUE(statement->decrementValue()->isSymbol());
			});
		}
		
		TEST(StatementParseTest, AssignConstant) {
			auto tokens = {
				Token::NAME,
				Token::SETEQUAL,
				Token::CONSTANT,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isAssign());
				EXPECT_EQ(statement->assignKind(), AST::ASSIGN_DIRECT);
				EXPECT_TRUE(statement->assignLvalue()->isSymbol());
				EXPECT_TRUE(statement->assignRvalue()->isLiteral());
			});
		}
		
		TEST(StatementParseTest, AddAssignConstant) {
			auto tokens = {
				Token::NAME,
				Token::ADDEQUAL,
				Token::CONSTANT,
				Token::SEMICOLON
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
				Token::CONSTANT,
				Token::SEMICOLON
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
				Token::CONSTANT,
				Token::SEMICOLON
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
				Token::CONSTANT,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isAssign());
				EXPECT_EQ(statement->assignKind(), AST::ASSIGN_DIV);
				EXPECT_TRUE(statement->assignLvalue()->isSymbol());
				EXPECT_TRUE(statement->assignRvalue()->isLiteral());
			});
		}
		
		TEST(StatementParseTest, ModuloAssignConstant) {
			auto tokens = {
				Token::NAME,
				Token::PERCENTEQUAL,
				Token::CONSTANT,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isAssign());
				EXPECT_EQ(statement->assignKind(), AST::ASSIGN_MOD);
				EXPECT_TRUE(statement->assignLvalue()->isSymbol());
				EXPECT_TRUE(statement->assignRvalue()->isLiteral());
			});
		}
		
		TEST(StatementParseTest, VarDeclNamedTypeAssignConstant) {
			auto tokens = {
				Token::NAME,
				Token::NAME,
				Token::SETEQUAL,
				Token::CONSTANT,
				Token::SEMICOLON
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
				Token::CONSTANT,
				Token::SEMICOLON
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
				Token::CONSTANT,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isVarDecl());
				ASSERT_TRUE(statement->varDeclVar()->isNamed());
				ASSERT_TRUE(statement->varDeclVar()->namedType()->isPointer());
				EXPECT_TRUE(statement->varDeclVar()->namedType()->getPointerTarget()->isObjectType());
				EXPECT_TRUE(statement->varDeclValue()->isLiteral());
			});
		}
		
		TEST(StatementParseTest, VarDeclPointerPointerTypeAssignConstant) {
			auto tokens = {
				Token::NAME,
				Token::STAR,
				Token::STAR,
				Token::NAME,
				Token::SETEQUAL,
				Token::CONSTANT,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isVarDecl());
				ASSERT_TRUE(statement->varDeclVar()->isNamed());
				ASSERT_TRUE(statement->varDeclVar()->namedType()->isPointer());
				EXPECT_TRUE(statement->varDeclVar()->namedType()->getPointerTarget()->isPointer());
				EXPECT_TRUE(statement->varDeclValue()->isLiteral());
			});
		}
		
		TEST(StatementParseTest, VarDeclIntPointerTypeAssignConstant) {
			auto tokens = {
				Token::INT,
				Token::STAR,
				Token::NAME,
				Token::SETEQUAL,
				Token::CONSTANT,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isVarDecl());
				ASSERT_TRUE(statement->varDeclVar()->isNamed());
				ASSERT_TRUE(statement->varDeclVar()->namedType()->isPointer());
				EXPECT_TRUE(statement->varDeclVar()->namedType()->getPointerTarget()->isPrimitive());
				EXPECT_TRUE(statement->varDeclValue()->isLiteral());
			});
		}
		
		TEST(StatementParseTest, VarDeclEmptyPatternMatch) {
			auto tokens = {
				Token::LET,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::SETEQUAL,
				Token::NAME,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isVarDecl());
				ASSERT_TRUE(statement->varDeclVar()->isPattern());
				EXPECT_EQ(statement->varDeclVar()->typeVarList()->size(), 0);
			});
		}
		
		TEST(StatementParseTest, VarDeclOneVarPatternMatch) {
			auto tokens = {
				Token::LET,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::NAME,
				Token::RROUNDBRACKET,
				Token::SETEQUAL,
				Token::NAME,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isVarDecl());
				ASSERT_TRUE(statement->varDeclVar()->isPattern());
				ASSERT_EQ(statement->varDeclVar()->typeVarList()->size(), 1);
				EXPECT_TRUE(statement->varDeclVar()->typeVarList()->at(0)->isNamed());
			});
		}
		
		TEST(StatementParseTest, VarDeclTwoVarsPatternMatch) {
			auto tokens = {
				Token::LET,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::NAME,
				Token::COMMA,
				Token::NAME,
				Token::NAME,
				Token::RROUNDBRACKET,
				Token::SETEQUAL,
				Token::NAME,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isVarDecl());
				ASSERT_TRUE(statement->varDeclVar()->isPattern());
				ASSERT_EQ(statement->varDeclVar()->typeVarList()->size(), 2);
				EXPECT_TRUE(statement->varDeclVar()->typeVarList()->at(0)->isNamed());
				EXPECT_TRUE(statement->varDeclVar()->typeVarList()->at(1)->isNamed());
			});
		}
		
		TEST(StatementParseTest, VarDeclIgnorePatternMatch) {
			auto tokens = {
				Token::LET,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::UNDERSCORE,
				Token::RROUNDBRACKET,
				Token::SETEQUAL,
				Token::NAME,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isVarDecl());
				ASSERT_TRUE(statement->varDeclVar()->isPattern());
				ASSERT_EQ(statement->varDeclVar()->typeVarList()->size(), 1);
				EXPECT_TRUE(statement->varDeclVar()->typeVarList()->at(0)->isAny());
			});
		}
		
		TEST(StatementParseTest, VarDeclOneVarAndIgnorePatternMatch) {
			auto tokens = {
				Token::LET,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::NAME,
				Token::COMMA,
				Token::UNDERSCORE,
				Token::RROUNDBRACKET,
				Token::SETEQUAL,
				Token::NAME,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isVarDecl());
				ASSERT_TRUE(statement->varDeclVar()->isPattern());
				ASSERT_EQ(statement->varDeclVar()->typeVarList()->size(), 2);
				EXPECT_TRUE(statement->varDeclVar()->typeVarList()->at(0)->isNamed());
				EXPECT_TRUE(statement->varDeclVar()->typeVarList()->at(1)->isAny());
			});
		}
		
		TEST(StatementParseTest, VarDeclOneIgnorePatternMatch) {
			auto tokens = {
				Token::LET,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::UNDERSCORE,
				Token::RROUNDBRACKET,
				Token::SETEQUAL,
				Token::NAME,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isVarDecl());
				ASSERT_TRUE(statement->varDeclVar()->isPattern());
				ASSERT_EQ(statement->varDeclVar()->typeVarList()->size(), 1);
				EXPECT_TRUE(statement->varDeclVar()->typeVarList()->at(0)->isAny());
			});
		}
		
		TEST(StatementParseTest, VarDeclTwoIgnorePatternMatch) {
			auto tokens = {
				Token::LET,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::UNDERSCORE,
				Token::COMMA,
				Token::UNDERSCORE,
				Token::RROUNDBRACKET,
				Token::SETEQUAL,
				Token::NAME,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isVarDecl());
				ASSERT_TRUE(statement->varDeclVar()->isPattern());
				ASSERT_EQ(statement->varDeclVar()->typeVarList()->size(), 2);
				EXPECT_TRUE(statement->varDeclVar()->typeVarList()->at(0)->isAny());
				EXPECT_TRUE(statement->varDeclVar()->typeVarList()->at(1)->isAny());
			});
		}
		
		TEST(StatementParseTest, AssertName) {
			auto tokens = {
				Token::ASSERT,
				Token::NAME,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isAssert());
				EXPECT_TRUE(statement->assertValue()->isSymbol());
			});
		}
		
		TEST(StatementParseTest, AssertComparison) {
			auto tokens = {
				Token::ASSERT,
				Token::NAME,
				Token::ISEQUAL,
				Token::NAME,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isAssert());
				EXPECT_TRUE(statement->assertValue()->isBinaryOp());
			});
		}
		
		TEST(StatementParseTest, AssertNoExcept) {
			auto tokens = {
				Token::ASSERT,
				Token::NOEXCEPT,
				Token::LCURLYBRACKET,
				Token::RETURN,
				Token::SEMICOLON,
				Token::RCURLYBRACKET
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isAssertNoExcept());
				EXPECT_EQ(statement->assertNoExceptScope()->size(), 1);
			});
		}
		
		TEST(StatementParseTest, Break) {
			auto tokens = {
				Token::BREAK,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				EXPECT_TRUE(statement->isBreak());
			});
		}
		
		TEST(StatementParseTest, Continue) {
			auto tokens = {
				Token::CONTINUE,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				EXPECT_TRUE(statement->isContinue());
			});
		}
		
		TEST(StatementParseTest, Unreachable) {
			auto tokens = {
				Token::UNREACHABLE,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				EXPECT_TRUE(statement->isUnreachable());
			});
		}
		
		TEST(StatementParseTest, Rethrow) {
			auto tokens = {
				Token::THROW,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				EXPECT_TRUE(statement->isRethrow());
			});
		}
		
		TEST(StatementParseTest, Throw) {
			auto tokens = {
				Token::THROW,
				Token::NAME,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isThrow());
				EXPECT_TRUE(statement->throwValue()->isSymbol());
			});
		}
		
		TEST(StatementParseTest, ReturnVoid) {
			auto tokens = {
				Token::RETURN,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				EXPECT_TRUE(statement->isReturnVoid());
			});
		}
		
		TEST(StatementParseTest, ReturnConstant) {
			auto tokens = {
				Token::RETURN,
				Token::CONSTANT,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isReturn());
				EXPECT_EQ(statement->returnValue()->kind(), AST::Value::LITERAL);
			});
		}
		
		TEST(StatementParseTest, ReturnType) {
			auto tokens = {
				Token::RETURN,
				Token::INT,
				Token::SEMICOLON
			};
			testParseStatement(tokens, [](const AST::Node<AST::Statement>& statement) {
				ASSERT_TRUE(statement->isReturn());
				EXPECT_EQ(statement->returnValue()->kind(), AST::Value::TYPEREF);
			});
		}
		
	}
	
}
