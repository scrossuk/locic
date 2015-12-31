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
		
	}
	
}
