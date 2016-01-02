#include "gtest/gtest.h"

#include <locic/Parser/NamespaceParser.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Support/StringHost.hpp>

#include "MockTokenSource.hpp"

namespace locic {
	
	namespace Parser {
		
		template <typename FnType>
		void testParseNamespace(const Array<Token::Kind, 16>& tokenKinds, FnType fn) {
			StringHost stringHost;
			MockTokenSource tokenSource(stringHost, tokenKinds);
			TokenReader tokenReader(tokenSource);
			const auto nameSpace = NamespaceParser(tokenReader).parseNamespace();
			EXPECT_TRUE(tokenSource.allConsumed());
			EXPECT_TRUE(tokenReader.peek().kind() == Token::END);
			fn(nameSpace);
		}
		
		template <typename FnType>
		void testParseGlobalNamespace(const Array<Token::Kind, 16>& tokenKinds, FnType fn) {
			StringHost stringHost;
			MockTokenSource tokenSource(stringHost, tokenKinds);
			TokenReader tokenReader(tokenSource);
			const auto nameSpace = NamespaceParser(tokenReader).parseGlobalNamespace();
			EXPECT_TRUE(tokenSource.allConsumed());
			EXPECT_TRUE(tokenReader.peek().kind() == Token::END);
			fn(nameSpace);
		}
		
		TEST(NamespaceParseTest, EmptyNamespace) {
			auto tokens = {
				Token::NAMESPACE,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::RCURLYBRACKET
			};
			testParseNamespace(tokens, [](const AST::Node<AST::NamespaceDecl>& nameSpace) {
				EXPECT_TRUE(nameSpace->data()->empty());
			});
		}
		
		TEST(NamespaceParseTest, EmptyClassDeclInNamespace) {
			auto tokens = {
				Token::NAMESPACE,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::CLASS,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::RCURLYBRACKET,
				Token::RCURLYBRACKET
			};
			testParseNamespace(tokens, [](const AST::Node<AST::NamespaceDecl>& nameSpace) {
				ASSERT_EQ(nameSpace->data()->typeInstances.size(), 1);
				EXPECT_EQ(nameSpace->data()->typeInstances[0]->variables->size(), 0);
				EXPECT_EQ(nameSpace->data()->typeInstances[0]->functions->size(), 0);
			});
		}
		
		TEST(NamespaceParseTest, EmptyClassDefInNamespace) {
			auto tokens = {
				Token::NAMESPACE,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::CLASS,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::LCURLYBRACKET,
				Token::RCURLYBRACKET,
				Token::RCURLYBRACKET
			};
			testParseNamespace(tokens, [](const AST::Node<AST::NamespaceDecl>& nameSpace) {
				ASSERT_EQ(nameSpace->data()->typeInstances.size(), 1);
				EXPECT_EQ(nameSpace->data()->typeInstances[0]->variables->size(), 0);
				EXPECT_EQ(nameSpace->data()->typeInstances[0]->functions->size(), 0);
			});
		}
		
		TEST(NamespaceParseTest, AliasInNamespace) {
			auto tokens = {
				Token::NAMESPACE,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::USING,
				Token::NAME,
				Token::SETEQUAL,
				Token::NAME,
				Token::SEMICOLON,
				Token::RCURLYBRACKET
			};
			testParseNamespace(tokens, [](const AST::Node<AST::NamespaceDecl>& nameSpace) {
				ASSERT_EQ(nameSpace->data()->aliases.size(), 1);
				EXPECT_TRUE(nameSpace->data()->aliases[0]->value()->isSymbol());
			});
		}
		
		TEST(NamespaceParseTest, TemplatedAliasInNamespace) {
			auto tokens = {
				Token::NAMESPACE,
				Token::NAME,
				Token::LCURLYBRACKET,
				
				Token::TEMPLATE,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::NAME,
				Token::RTRIBRACKET,
				
				Token::USING,
				Token::NAME,
				Token::SETEQUAL,
				Token::NAME,
				Token::SEMICOLON,
				
				Token::RCURLYBRACKET
			};
			testParseNamespace(tokens, [](const AST::Node<AST::NamespaceDecl>& nameSpace) {
				ASSERT_EQ(nameSpace->data()->aliases.size(), 1);
				EXPECT_TRUE(nameSpace->data()->aliases[0]->value()->isSymbol());
			});
		}
		
		TEST(NamespaceParseTest, TemplatedAliasWithRequireInNamespace) {
			auto tokens = {
				Token::NAMESPACE,
				Token::NAME,
				Token::LCURLYBRACKET,
				
				Token::TEMPLATE,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::NAME,
				Token::RTRIBRACKET,
				
				Token::REQUIRE,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET,
				
				Token::USING,
				Token::NAME,
				Token::SETEQUAL,
				Token::NAME,
				Token::SEMICOLON,
				
				Token::RCURLYBRACKET
			};
			testParseNamespace(tokens, [](const AST::Node<AST::NamespaceDecl>& nameSpace) {
				ASSERT_EQ(nameSpace->data()->aliases.size(), 1);
				EXPECT_TRUE(nameSpace->data()->aliases[0]->value()->isSymbol());
			});
		}
		
		TEST(NamespaceParseTest, TemplatedClassWithNoTagSetInNamespace) {
			auto tokens = {
				Token::NAMESPACE,
				Token::NAME,
				Token::LCURLYBRACKET,
				
				Token::TEMPLATE,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::NAME,
				Token::RTRIBRACKET,
				
				Token::NOTAG,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::RROUNDBRACKET,
				
				Token::CLASS,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::RCURLYBRACKET,
				
				Token::RCURLYBRACKET
			};
			testParseNamespace(tokens, [](const AST::Node<AST::NamespaceDecl>& nameSpace) {
				ASSERT_EQ(nameSpace->data()->typeInstances.size(), 1);
			});
		}
		
		TEST(NamespaceParseTest, TemplatedPrimitiveInNamespace) {
			auto tokens = {
				Token::NAMESPACE,
				Token::NAME,
				Token::LCURLYBRACKET,
				
				Token::TEMPLATE,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::NAME,
				Token::RTRIBRACKET,
				
				Token::PRIMITIVE,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::RCURLYBRACKET,
				
				Token::RCURLYBRACKET
			};
			testParseNamespace(tokens, [](const AST::Node<AST::NamespaceDecl>& nameSpace) {
				ASSERT_EQ(nameSpace->data()->typeInstances.size(), 1);
				EXPECT_EQ(nameSpace->data()->typeInstances[0]->kind, AST::TypeInstance::PRIMITIVE);
			});
		}
		
		TEST(NamespaceParseTest, ImportGlobalFunctionDecl) {
			auto tokens = {
				Token::NAMESPACE,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::IMPORT,
				Token::NAME,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::SEMICOLON,
				Token::RCURLYBRACKET
			};
			testParseNamespace(tokens, [](const AST::Node<AST::NamespaceDecl>& nameSpace) {
				ASSERT_EQ(nameSpace->data()->functions.size(), 1);
			});
		}
		
	}
	
}
