#include "gtest/gtest.h"

#include <locic/Constant.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TokenSource.hpp>
#include <locic/Parser/ValueParser.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/StringHost.hpp>

namespace locic {
	
	namespace Parser {
		
		class MockTokenSource: public TokenSource {
		public:
			MockTokenSource(const StringHost& stringHost,
			                const Array<Token, 16>& tokens)
			: stringHost_(stringHost), tokens_(tokens),
			position_(0) { }
			
			Token get() {
				if (position_ == tokens_.size()) {
					return Token::Basic(Token::END);
				} else {
					return tokens_[position_++];
				}
			}
			
			String fileName() {
				return String(stringHost_, "<test file>");
			}
			
			bool allConsumed() const {
				return position_ == tokens_.size();
			}
			
		private:
			const StringHost& stringHost_;
			const Array<Token, 16>& tokens_;
			size_t position_;
			
		};
		
		class TokenGenerator {
		public:
			TokenGenerator(const StringHost& stringHost)
			: stringHost_(stringHost), nextInteger_(0) { }
			
			Token makeToken(const Token::Kind kind) {
				switch (kind) {
					case Token::NAME:
						return Token::Name(String(stringHost_, "test"));
					case Token::CONSTANT:
						return Token::Constant(Constant::Integer(nextInteger_++));
					case Token::VERSION:
						return Token::Version(Version(1, 0, 0));
					default:
						return Token::Basic(kind);
				}
			}
			
		private:
			const StringHost& stringHost_;
			unsigned long long nextInteger_;
			
		};
		
		template <typename FnType>
		void testParseValue(const Array<Token::Kind, 16>& tokenKinds, FnType fn) {
			StringHost stringHost;
			
			TokenGenerator tokenGenerator(stringHost);
			
			Array<Token, 16> tokens;
			for (const auto tokenKind: tokenKinds) {
				tokens.push_back(tokenGenerator.makeToken(tokenKind));
			}
			
			MockTokenSource tokenSource(stringHost, tokens);
			TokenReader tokenReader(tokenSource);
			const auto value = ValueParser(tokenReader).parseValue();
			EXPECT_TRUE(tokenSource.allConsumed());
			fn(value);
		}
		
		TEST(ValueParseTest, Literal) {
			testParseValue({ Token::CONSTANT }, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::LITERAL);
			});
		}
		
		TEST(ValueParseTest, Self) {
			testParseValue({ Token::SELF }, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::SELF);
			});
		}
		
		TEST(ValueParseTest, This) {
			testParseValue({ Token::THIS }, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::THIS);
			});
		}
		
		TEST(ValueParseTest, LeftShift) {
			auto tokens = {
				Token::NAME,
				Token::DOUBLE_LTRIBRACKET,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_LEFTSHIFT);
				EXPECT_EQ(value->binaryOp.leftOperand->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->binaryOp.rightOperand->kind(), AST::Value::SYMBOLREF);
			});
		}
		
		TEST(ValueParseTest, RightShift) {
			auto tokens = {
				Token::NAME,
				Token::RTRIBRACKET,
				Token::RTRIBRACKET,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_RIGHTSHIFT);
				EXPECT_EQ(value->binaryOp.leftOperand->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->binaryOp.rightOperand->kind(), AST::Value::SYMBOLREF);
			});
		}
		
		TEST(ValueParseTest, Add) {
			auto tokens = {
				Token::NAME,
				Token::PLUS,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_ADD);
			});
		}
		
		TEST(ValueParseTest, AddLeftAssociative) {
			auto tokens = {
				Token::NAME,
				Token::PLUS,
				Token::NAME,
				Token::PLUS,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_ADD);
				EXPECT_EQ(value->binaryOp.leftOperand->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.leftOperand->binaryOp.kind, AST::OP_ADD);
				EXPECT_EQ(value->binaryOp.rightOperand->kind(), AST::Value::SYMBOLREF);
			});
		}
		
		TEST(ValueParseTest, MultiplyInAddLeft) {
			auto tokens = {
				Token::NAME,
				Token::STAR,
				Token::NAME,
				Token::PLUS,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_ADD);
				EXPECT_EQ(value->binaryOp.leftOperand->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.leftOperand->binaryOp.kind, AST::OP_MULTIPLY);
				EXPECT_EQ(value->binaryOp.rightOperand->kind(), AST::Value::SYMBOLREF);
			});
		}
		
		TEST(ValueParseTest, MultiplyInAddRight) {
			auto tokens = {
				Token::NAME,
				Token::PLUS,
				Token::NAME,
				Token::STAR,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.kind, AST::OP_ADD);
				EXPECT_EQ(value->binaryOp.leftOperand->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->binaryOp.rightOperand->kind(), AST::Value::BINARYOP);
				EXPECT_EQ(value->binaryOp.rightOperand->binaryOp.kind, AST::OP_MULTIPLY);
			});
		}
		
		TEST(ValueParseTest, SymbolTwoComponents) {
			auto tokens = {
				Token::NAME,
				Token::DOUBLE_COLON,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->symbolRef.symbol->size(), 2);
				EXPECT_EQ(value->symbolRef.symbol->at(0)->templateArguments()->size(), 0);
				EXPECT_EQ(value->symbolRef.symbol->at(1)->templateArguments()->size(), 0);
			});
		}
		
		TEST(ValueParseTest, SymbolThreeComponents) {
			auto tokens = {
				Token::NAME,
				Token::DOUBLE_COLON,
				Token::NAME,
				Token::DOUBLE_COLON,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->symbolRef.symbol->size(), 3);
				EXPECT_EQ(value->symbolRef.symbol->at(0)->templateArguments()->size(), 0);
				EXPECT_EQ(value->symbolRef.symbol->at(1)->templateArguments()->size(), 0);
				EXPECT_EQ(value->symbolRef.symbol->at(2)->templateArguments()->size(), 0);
			});
		}
		
		TEST(ValueParseTest, SymbolTemplatedChild) {
			auto tokens = {
				Token::NAME,
				Token::DOUBLE_COLON,
				Token::NAME,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::RTRIBRACKET
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->symbolRef.symbol->size(), 2);
				EXPECT_EQ(value->symbolRef.symbol->at(0)->templateArguments()->size(), 0);
				EXPECT_EQ(value->symbolRef.symbol->at(1)->templateArguments()->size(), 1);
			});
		}
		
		TEST(ValueParseTest, SymbolTemplatedParent) {
			auto tokens = {
				Token::NAME,
				Token::LTRIBRACKET,
				Token::NAME,
				Token::RTRIBRACKET,
				Token::DOUBLE_COLON,
				Token::NAME
			};
			testParseValue(tokens, [](const AST::Node<AST::Value>& value) {
				EXPECT_EQ(value->kind(), AST::Value::SYMBOLREF);
				EXPECT_EQ(value->symbolRef.symbol->size(), 2);
				EXPECT_EQ(value->symbolRef.symbol->at(0)->templateArguments()->size(), 1);
				EXPECT_EQ(value->symbolRef.symbol->at(1)->templateArguments()->size(), 0);
			});
		}
		
	}
	
}