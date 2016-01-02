#include "gtest/gtest.h"

#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeInstanceParser.hpp>
#include <locic/Support/StringHost.hpp>

#include "MockTokenSource.hpp"

namespace locic {
	
	namespace Parser {
		
		template <typename FnType>
		void testParseTypeInstance(const Array<Token::Kind, 16>& tokenKinds, FnType fn) {
			StringHost stringHost;
			MockTokenSource tokenSource(stringHost, tokenKinds);
			TokenReader tokenReader(tokenSource);
			const auto typeInstance = TypeInstanceParser(tokenReader).parseTypeInstance();
			EXPECT_TRUE(tokenSource.allConsumed());
			EXPECT_TRUE(tokenReader.peek().kind() == Token::END);
			fn(typeInstance);
		}
		
		TEST(TypeInstanceParseTest, EmptyClassDecl) {
			auto tokens = {
				Token::CLASS,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::CLASSDECL);
				EXPECT_EQ(typeInstance->functions->size(), 0);
				EXPECT_EQ(typeInstance->variables->size(), 0);
			});
		}
		
		TEST(TypeInstanceParseTest, ClassDeclMethod) {
			auto tokens = {
				Token::CLASS,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::NAME,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::SEMICOLON,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::CLASSDECL);
				EXPECT_EQ(typeInstance->functions->size(), 1);
				EXPECT_EQ(typeInstance->variables->size(), 0);
			});
		}
		
		TEST(TypeInstanceParseTest, ClassDeclNullMethod) {
			auto tokens = {
				Token::CLASS,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::NAME,
				Token::NULLVAL,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::SEMICOLON,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::CLASSDECL);
				EXPECT_EQ(typeInstance->functions->size(), 1);
				EXPECT_EQ(typeInstance->variables->size(), 0);
			});
		}
		
		TEST(TypeInstanceParseTest, ClassDeclMethodTemplatedReturnType) {
			auto tokens = {
				Token::CLASS,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::NAME,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::SEMICOLON,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::CLASSDECL);
				EXPECT_EQ(typeInstance->functions->size(), 1);
				EXPECT_EQ(typeInstance->variables->size(), 0);
			});
		}
		
		TEST(TypeInstanceParseTest, TemplatedClassDeclMethod) {
			auto tokens = {
				Token::CLASS,
				Token::NAME,
				Token::LCURLYBRACKET,
				
				Token::TEMPLATE,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::NAME,
				Token::RTRIBRACKET,
				
				Token::NAME,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::SEMICOLON,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::CLASSDECL);
				EXPECT_EQ(typeInstance->functions->size(), 1);
				EXPECT_EQ(typeInstance->variables->size(), 0);
			});
		}
		
		TEST(TypeInstanceParseTest, EmptyClassDef) {
			auto tokens = {
				Token::CLASS,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::LCURLYBRACKET,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::CLASSDEF);
				EXPECT_EQ(typeInstance->functions->size(), 0);
				EXPECT_EQ(typeInstance->variables->size(), 0);
			});
		}
		
		TEST(TypeInstanceParseTest, ClassDefMethod) {
			auto tokens = {
				Token::CLASS,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::LCURLYBRACKET,
				Token::NAME,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::LCURLYBRACKET,
				Token::RCURLYBRACKET,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::CLASSDEF);
				EXPECT_EQ(typeInstance->functions->size(), 1);
				EXPECT_EQ(typeInstance->variables->size(), 0);
			});
		}
		
		TEST(TypeInstanceParseTest, TemplatedClassDefMethod) {
			auto tokens = {
				Token::CLASS,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::LCURLYBRACKET,
				
				Token::TEMPLATE,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::NAME,
				Token::RTRIBRACKET,
				
				Token::NAME,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::LCURLYBRACKET,
				Token::RCURLYBRACKET,
				
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::CLASSDEF);
				EXPECT_EQ(typeInstance->functions->size(), 1);
				EXPECT_EQ(typeInstance->variables->size(), 0);
			});
		}
		
		TEST(TypeInstanceParseTest, ClassDefOneVariable) {
			auto tokens = {
				Token::CLASS,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::NAME,
				Token::RROUNDBRACKET,
				Token::LCURLYBRACKET,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::CLASSDEF);
				EXPECT_EQ(typeInstance->functions->size(), 0);
				EXPECT_EQ(typeInstance->variables->size(), 1);
			});
		}
		
		TEST(TypeInstanceParseTest, ClassDefTwoVariables) {
			auto tokens = {
				Token::CLASS,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::NAME,
				Token::NAME,
				Token::COMMA,
				Token::NAME,
				Token::NAME,
				Token::RROUNDBRACKET,
				Token::LCURLYBRACKET,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::CLASSDEF);
				EXPECT_EQ(typeInstance->functions->size(), 0);
				EXPECT_EQ(typeInstance->variables->size(), 2);
			});
		}
		
		TEST(TypeInstanceParseTest, EmptyInterface) {
			auto tokens = {
				Token::INTERFACE,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::INTERFACE);
				EXPECT_EQ(typeInstance->functions->size(), 0);
			});
		}
		
		TEST(TypeInstanceParseTest, InterfaceMethod) {
			auto tokens = {
				Token::INTERFACE,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::NAME,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::SEMICOLON,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::INTERFACE);
				EXPECT_EQ(typeInstance->functions->size(), 1);
			});
		}
		
		TEST(TypeInstanceParseTest, TemplatedInterfaceMethod) {
			auto tokens = {
				Token::INTERFACE,
				Token::NAME,
				Token::LCURLYBRACKET,
				
				Token::TEMPLATE,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::NAME,
				Token::RTRIBRACKET,
				
				Token::NAME,
				Token::NAME,
				Token::LROUNDBRACKET,
				Token::RROUNDBRACKET,
				Token::SEMICOLON,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::INTERFACE);
				EXPECT_EQ(typeInstance->functions->size(), 1);
			});
		}
		
		TEST(TypeInstanceParseTest, EmptyStruct) {
			auto tokens = {
				Token::STRUCT,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::STRUCT);
				EXPECT_EQ(typeInstance->variables->size(), 0);
			});
		}
		
		TEST(TypeInstanceParseTest, StructOneVariable) {
			auto tokens = {
				Token::STRUCT,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::NAME,
				Token::NAME,
				Token::SEMICOLON,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::STRUCT);
				EXPECT_EQ(typeInstance->variables->size(), 1);
			});
		}
		
		TEST(TypeInstanceParseTest, StructTwoVariables) {
			auto tokens = {
				Token::STRUCT,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::NAME,
				Token::NAME,
				Token::SEMICOLON,
				Token::NAME,
				Token::NAME,
				Token::SEMICOLON,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::STRUCT);
				EXPECT_EQ(typeInstance->variables->size(), 2);
			});
		}
		
		TEST(TypeInstanceParseTest, EmptyUnion) {
			auto tokens = {
				Token::UNION,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::UNION);
				EXPECT_EQ(typeInstance->variables->size(), 0);
			});
		}
		
		TEST(TypeInstanceParseTest, UnionOneVariable) {
			auto tokens = {
				Token::UNION,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::NAME,
				Token::NAME,
				Token::SEMICOLON,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::UNION);
				EXPECT_EQ(typeInstance->variables->size(), 1);
			});
		}
		
		TEST(TypeInstanceParseTest, UnionTwoVariables) {
			auto tokens = {
				Token::UNION,
				Token::NAME,
				Token::LCURLYBRACKET,
				Token::NAME,
				Token::NAME,
				Token::SEMICOLON,
				Token::NAME,
				Token::NAME,
				Token::SEMICOLON,
				Token::RCURLYBRACKET
			};
			testParseTypeInstance(tokens, [](const AST::Node<AST::TypeInstance>& typeInstance) {
				EXPECT_EQ(typeInstance->kind, AST::TypeInstance::UNION);
				EXPECT_EQ(typeInstance->variables->size(), 2);
			});
		}
		
	}
	
}
